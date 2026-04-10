package autosave

import (
	"os"
	"testing"
)

func TestAutosaveManager(t *testing.T) {
	tempDir, err := os.MkdirTemp("", "npp_autosave_test")
	if err != nil {
		t.Fatalf("Failed to create temp dir: %v", err)
	}
	defer os.RemoveAll(tempDir)

	manager, err := NewManager(tempDir)
	if err != nil {
		t.Fatalf("Failed to create manager: %v", err)
	}

	testFile := "C:\\projects\\test.txt"
	content1 := []byte("Initial content")
	content2 := []byte("Updated content")

	// Save snapshot 1
	path1, err := manager.SaveSnapshot(testFile, content1)
	if err != nil || path1 == "" {
		t.Fatalf("Failed to save snapshot 1: %v", err)
	}

	// Save snapshot 2
	path2, err := manager.SaveSnapshot(testFile, content2)
	if err != nil || path2 == "" {
		t.Fatalf("Failed to save snapshot 2: %v", err)
	}

	// Verify history
	history, err := manager.GetHistory(testFile)
	if err != nil {
		t.Fatalf("Failed to get history: %v", err)
	}

	if len(history) != 2 {
		t.Errorf("Expected 2 snapshots in history, got %d", len(history))
	}
}
