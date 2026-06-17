package lsp

import (
	"context"
	"encoding/json"
	"fmt"
	"strings"
	"sync"
	"sync/atomic"
	"time"

	"github.com/notepad-plus-plus/ultra-project/pkg/core"
)

// Deep comment: Manager correlates active UI Buffers with specific Language Servers.
// Why: If a user has a .go file and a .py file open side-by-side, we must route
//      the autocomplete requests to `gopls` and `pyright` respectively.
//      This registry maintains the active, long-lived server processes.

type Manager struct {
	mu      sync.RWMutex
	servers map[string]*Client // Keyed by language (e.g., "golang")
	ctx     context.Context
	nextID  atomic.Int64
}

func NewManager(ctx context.Context) *Manager {
	return &Manager{
		servers: make(map[string]*Client),
		ctx:     ctx,
	}
}

// StartServer boots a new LSP for a given language if one is not already running.
func (m *Manager) StartServer(language string, command string, args ...string) error {
	m.mu.Lock()
	defer m.mu.Unlock()

	if _, exists := m.servers[language]; exists {
		return nil // Already running
	}

	client := NewClient(language, command, args...)
	if err := client.Start(m.ctx); err != nil {
		return fmt.Errorf("failed to start LSP server for %s: %w", language, err)
	}

	m.servers[language] = client
	return nil
}

// AddClient injects a pre-configured client (used for testing with mock servers).
func (m *Manager) AddClient(language string, client *Client) {
	m.mu.Lock()
	defer m.mu.Unlock()
	m.servers[language] = client
}

// RequestCompletion asks the appropriate LSP for code suggestions at a specific cursor position.
// It sends a textDocument/completion JSON-RPC request and parses the response.
func (m *Manager) RequestCompletion(buf *core.Buffer, line, character int) ([]string, error) {
	m.mu.RLock()
	client, exists := m.servers[buf.LanguageType]
	m.mu.RUnlock()

	if !exists {
		return nil, fmt.Errorf("no language server configured for %s", buf.LanguageType)
	}

	// Build a file:// URI from the buffer's filepath
	uri := fmt.Sprintf("file:///%s", strings.ReplaceAll(buf.Filepath, "\\", "/"))

	// Create completion params
	params := CompletionParams{
		TextDocument: TextDocumentIdentifier{URI: uri},
		Position:     Position{Line: line, Character: character},
	}

	paramsJSON, err := json.Marshal(params)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal completion params: %w", err)
	}

	// Build JSON-RPC request
	reqID := m.nextID.Add(1)
	req := Request{
		JSONRPC: "2.0",
		ID:      reqID,
		Method:  "textDocument/completion",
		Params:  paramsJSON,
	}

	// Write the request to the LSP server's stdin
	if err := client.WriteMessage(req); err != nil {
		return nil, fmt.Errorf("failed to write completion request: %w", err)
	}

	// Read the response (with timeout)
	type respResult struct {
		resp *Response
		err  error
	}
	resultCh := make(chan respResult, 1)

	go func() {
		raw, err := client.ReadMessage()
		if err != nil {
			resultCh <- respResult{err: err}
			return
		}

		var resp Response
		if err := json.Unmarshal(raw, &resp); err != nil {
			resultCh <- respResult{err: fmt.Errorf("failed to parse LSP response JSON: %w", err)}
			return
		}
		resultCh <- respResult{resp: &resp}
	}()

	var r respResult
	select {
	case r = <-resultCh:
	case <-time.After(10 * time.Second):
		return nil, fmt.Errorf("completion request timed out after 10s")
	}

	if r.err != nil {
		return nil, fmt.Errorf("failed to read LSP response: %w", r.err)
	}

	if r.resp.Error != nil {
		return nil, r.resp.Error
	}

	// Parse result — LSP spec says result is CompletionList | []CompletionItem
	var completionList CompletionList
	if err := json.Unmarshal(r.resp.Result, &completionList); err != nil {
		// Try as plain []CompletionItem array
		var items []CompletionItem
		if err2 := json.Unmarshal(r.resp.Result, &items); err2 != nil {
			return nil, fmt.Errorf("failed to parse completion result: as list: %v; as array: %v", err, err2)
		}
		completionList.Items = items
	}

	// Extract labels
	labels := make([]string, len(completionList.Items))
	for i, item := range completionList.Items {
		labels[i] = item.Label
	}

	return labels, nil
}

// ShutdownAll gracefully kills all active language servers.
func (m *Manager) ShutdownAll() {
	m.mu.Lock()
	defer m.mu.Unlock()

	for _, client := range m.servers {
		client.Stop()
	}
	m.servers = make(map[string]*Client)
}
