package lsp

import (
	"context"
	"testing"

	"github.com/notepad-plus-plus/ultra-project/pkg/core"
)

func TestLSPManager(t *testing.T) {
	ctx := context.Background()
	mgr := NewManager(ctx)
	defer mgr.ShutdownAll()

	// Use `cat` as a mock cross-platform executable
	err := mgr.StartServer("golang", "cat")
	if err != nil {
		t.Fatalf("Failed to start mock server: %v", err)
	}

	buf := &core.Buffer{LanguageType: "golang"}

	completions, err := mgr.RequestCompletion(buf, 1, 10)
	if err != nil {
		t.Fatalf("Failed to request completions: %v", err)
	}

	if len(completions) != 2 || completions[0] != "StubCompletion1" {
		t.Errorf("Expected 2 mock completions, got %v", completions)
	}

	// Try unsupported language
	badBuf := &core.Buffer{LanguageType: "python"}
	_, err = mgr.RequestCompletion(badBuf, 1, 10)
	if err == nil {
		t.Error("Expected error requesting completion on unconfigured language")
	}
}
