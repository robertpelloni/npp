package io

import (
	"os"
	"path/filepath"
	"testing"
)

func TestFileManager(t *testing.T) {
	fm := NewFileManager()

	tempDir, err := os.MkdirTemp("", "npp_fileio_test")
	if err != nil {
		t.Fatalf("Failed to create temp dir: %v", err)
	}
	defer os.RemoveAll(tempDir)

	testPath := filepath.Join(tempDir, "test.txt")
	content := []byte("Hello, File I/O Port!")

	// Test Write
	if err := fm.WriteFile(testPath, content); err != nil {
		t.Fatalf("Failed to write test file: %v", err)
	}

	// Test Read
	readBack, err := fm.ReadFile(testPath)
	if err != nil {
		t.Fatalf("Failed to read test file: %v", err)
	}

	if string(readBack) != string(content) {
		t.Errorf("Expected %q, got %q", string(content), string(readBack))
	}
}
