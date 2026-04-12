package lsp

import (
	"context"
	"fmt"
	"sync"

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

// RequestCompletion asks the appropriate LSP for code suggestions at a specific cursor position.
// Note: This is an async stub. Real JSON-RPC requires sequence IDs and listening on stdout.
func (m *Manager) RequestCompletion(buf *core.Buffer, line, character int) ([]string, error) {
	m.mu.RLock()
	client, exists := m.servers[buf.LanguageType]
	m.mu.RUnlock()

	if !exists {
		return nil, fmt.Errorf("no language server configured for %s", buf.LanguageType)
	}

	// Stub:
	// 1. Format `textDocument/completion` JSON-RPC payload.
	// 2. Write to `client.in`.
	// 3. Await response channel reading from `client.out`.
	_ = client
	return []string{"StubCompletion1", "StubCompletion2"}, nil
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
