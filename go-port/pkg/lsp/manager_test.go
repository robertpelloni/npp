package lsp

import (
	"bufio"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"strings"
	"testing"
	"time"

	"github.com/notepad-plus-plus/ultra-project/pkg/core"
)

// mockLSPServer creates a goroutine-based mock LSP server and returns a Client connected to it.
// The server reads JSON-RPC requests from its inbound pipe and writes responses to its outbound pipe.
func mockLSPServer(t *testing.T, language string, handler func(req Request) Response) *Client {
	t.Helper()

	// clientToServer: client writes requests, server reads them
	clientToServerReader, clientToServerWriter := io.Pipe()
	// serverToClient: server writes responses, client reads them
	serverToClientReader, serverToClientWriter := io.Pipe()

	// Goroutine acts as the LSP server
	go func() {
		br := bufio.NewReader(clientToServerReader)
		for {
			// Read Content-Length header
			var contentLength int
			for {
				line, err := br.ReadString('\n')
				if err != nil {
					if err != io.EOF {
						t.Logf("[mock-lsp] header read error: %v", err)
					}
					_ = serverToClientWriter.Close()
					return
				}
				line = strings.TrimRight(line, "\r\n")
				if line == "" {
					break // end of headers
				}
				if strings.HasPrefix(line, "Content-Length:") {
					parts := strings.SplitN(line, ":", 2)
					if len(parts) == 2 {
						fmt.Sscanf(strings.TrimSpace(parts[1]), "%d", &contentLength)
					}
				}
			}

			if contentLength <= 0 {
				t.Log("[mock-lsp] invalid Content-Length, closing")
				_ = serverToClientWriter.Close()
				return
			}

			// Read body
			body := make([]byte, contentLength)
			if _, err := io.ReadFull(br, body); err != nil {
				t.Logf("[mock-lsp] body read error: %v", err)
				_ = serverToClientWriter.Close()
				return
			}

			// Parse request
			var req Request
			if err := json.Unmarshal(body, &req); err != nil {
				t.Logf("[mock-lsp] failed to parse request: %v", err)
				continue
			}

			t.Logf("[mock-lsp] received method=%s id=%v", req.Method, req.ID)

			// Generate response
			resp := handler(req)

			// Write response with headers
			respJSON, err := json.Marshal(resp)
			if err != nil {
				t.Logf("[mock-lsp] marshal error: %v", err)
				continue
			}

			header := fmt.Sprintf("Content-Length: %d\r\n\r\n", len(respJSON))
			if _, err := serverToClientWriter.Write([]byte(header)); err != nil {
				t.Logf("[mock-lsp] header write error: %v", err)
				return
			}
			if _, err := serverToClientWriter.Write(respJSON); err != nil {
				t.Logf("[mock-lsp] body write error: %v", err)
				return
			}
		}
	}()

	// Create a Client with the pipe ends
	// Client writes to clientToServerWriter, reads from serverToClientReader
	client := &Client{
		Language: language,
		Command:  "mock",
		Args:     nil,
		in:       clientToServerWriter,
		out:      bufio.NewReader(serverToClientReader),
		cancel:   func() {},
	}

	return client
}

func TestLSPCompletion(t *testing.T) {
	ctx := context.Background()
	mgr := NewManager(ctx)
	defer mgr.ShutdownAll()

	// Create mock server that responds to textDocument/completion with real results
	client := mockLSPServer(t, "golang", func(req Request) Response {
		if req.Method == "textDocument/completion" {
			items := []CompletionItem{
				{Label: "fmt.Println", Kind: 2, Detail: "func(a ...any) (n int, err error)"},
				{Label: "fmt.Sprintf", Kind: 2, Detail: "func(format string, a ...any) string"},
				{Label: "len", Kind: 2, Detail: "func(v Type) int"},
			}
			result := CompletionList{IsIncomplete: false, Items: items}
			resultJSON, _ := json.Marshal(result)
			return Response{
				JSONRPC: "2.0",
				ID:      req.ID,
				Result:  resultJSON,
			}
		}
		// Unknown method — return error
		errJSON, _ := json.Marshal(RPCError{Code: -32601, Message: "Method not found"})
		return Response{
			JSONRPC: "2.0",
			ID:      req.ID,
			Error:   &RPCError{Code: -32601, Message: "Method not found"},
			Result:  errJSON,
		}
	})

	mgr.AddClient("golang", client)

	buf := &core.Buffer{
		Filepath:     "/home/user/main.go",
		LanguageType: "golang",
		Content:      []byte("package main\n\nfunc main() {\n\tfmt.P\n}"),
	}

	completions, err := mgr.RequestCompletion(buf, 3, 6)
	if err != nil {
		t.Fatalf("RequestCompletion failed: %v", err)
	}

	if len(completions) != 3 {
		t.Fatalf("expected 3 completions, got %d: %v", len(completions), completions)
	}

	if completions[0] != "fmt.Println" {
		t.Errorf("expected first completion 'fmt.Println', got %q", completions[0])
	}

	if completions[1] != "fmt.Sprintf" {
		t.Errorf("expected second completion 'fmt.Sprintf', got %q", completions[1])
	}

	// Test unsupported language
	badBuf := &core.Buffer{LanguageType: "python"}
	_, err = mgr.RequestCompletion(badBuf, 0, 0)
	if err == nil {
		t.Error("expected error for unsupported language")
	}
}

func TestLSPCompletionTimeout(t *testing.T) {
	ctx := context.Background()
	mgr := NewManager(ctx)
	defer mgr.ShutdownAll()

	// Create a mock server that never responds (to test timeout)
	// Use pipes but don't start a server goroutine - so reads will block
	clientToServerReader, clientToServerWriter := io.Pipe()
	serverToClientReader, _ := io.Pipe()

	client := &Client{
		Language: "timeout-lang",
		Command:  "mock",
		Args:     nil,
		in:       clientToServerWriter,
		out:      bufio.NewReader(serverToClientReader),
		cancel:   func() {},
	}

	// Close the reader end to simulate a dead server
	_ = clientToServerReader.Close()
	_ = serverToClientReader.Close()

	mgr.AddClient("timeout-lang", client)

	buf := &core.Buffer{LanguageType: "timeout-lang"}

	// Request should timeout (10s timeout in manager.go)
	_, err := mgr.RequestCompletion(buf, 0, 0)
	if err == nil {
		t.Error("expected timeout or read error")
	}
}

func TestLSPCompletionErrorResponse(t *testing.T) {
	ctx := context.Background()
	mgr := NewManager(ctx)
	defer mgr.ShutdownAll()

	// Server that returns an LSP error
	client := mockLSPServer(t, "error-lang", func(req Request) Response {
		errJSON, _ := json.Marshal(RPCError{
			Code:    -32603,
			Message: "Internal error",
		})
		return Response{
			JSONRPC: "2.0",
			ID:      req.ID,
			Error:   &RPCError{Code: -32603, Message: "Internal error"},
			Result:  errJSON,
		}
	})

	mgr.AddClient("error-lang", client)

	buf := &core.Buffer{LanguageType: "error-lang"}
	_, err := mgr.RequestCompletion(buf, 0, 0)
	if err == nil {
		t.Error("expected error from LSP server")
	}
	if err != nil && !strings.Contains(err.Error(), "Internal error") {
		t.Errorf("expected 'Internal error', got: %v", err)
	}
}

func TestLSPManagerStartStop(t *testing.T) {
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	mgr := NewManager(ctx)
	defer mgr.ShutdownAll()

	// Use "cat" as a simple executable to test start/stop lifecycle
	err := mgr.StartServer("mock", "cat")
	if err != nil {
		t.Fatalf("StartServer failed: %v", err)
	}

	// Starting same language again should be a no-op
	err = mgr.StartServer("mock", "cat")
	if err != nil {
		t.Errorf("re-StartServer should succeed: %v", err)
	}
}
