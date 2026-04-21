package lsp

import (
	"context"
	"testing"
	"time"
)

func TestLSPClientStartStop(t *testing.T) {
	// We test the lifecycle with `cat` which mimics a PTY/STDIN pipe
	// just long enough to verify `exec.Cmd` piping works cleanly.
	client := NewClient("mock_lang", "cat")

	ctx, cancel := context.WithTimeout(context.Background(), 2*time.Second)
	defer cancel()

	if err := client.Start(ctx); err != nil {
		t.Fatalf("Failed to start mock LSP client: %v", err)
	}

	if client.cmd == nil || client.in == nil || client.out == nil {
		t.Error("Expected cmd and IO pipes to be initialized")
	}

	if err := client.Stop(); err != nil {
		t.Fatalf("Failed to stop mock LSP client: %v", err)
	}
}
