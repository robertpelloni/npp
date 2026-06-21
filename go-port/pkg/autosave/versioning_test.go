package autosave

import (
	"database/sql"
	"os"
	"testing"
	"time"

	_ "github.com/mattn/go-sqlite3"
	"github.com/notepad-plus-plus/ultra-project/pkg/core"
)

func TestVersionHistoryManager(t *testing.T) {
	if os.Getenv("CGO_ENABLED") == "0" {
		t.Skip("Skipping SQLite tests; CGO is disabled")
	}

	// Setup an in-memory DB
	db, err := sql.Open("sqlite3", ":memory:")
	if err != nil {
		t.Fatalf("Failed to open DB: %v", err)
	}
	defer db.Close()

	vhm := NewVersionHistoryManager(db)
	err = vhm.CreateSchema()
	if err != nil {
		t.Fatalf("Failed to create schema: %v", err)
	}

	// 1. Snapshot a new buffer
	buf := &core.Buffer{
		ID:       "test1",
		Filepath: "/test/file.txt",
		Content:  []byte("Version 1"),
	}

	err = vhm.SnapshotBuffer(buf)
	if err != nil {
		t.Errorf("SnapshotBuffer failed: %v", err)
	}

	// 2. Snapshot the SAME buffer (should skip)
	err = vhm.SnapshotBuffer(buf)
	if err != nil {
		t.Errorf("SnapshotBuffer failed on duplicate: %v", err)
	}

	// Verify only 1 snapshot exists
	history, err := vhm.GetHistory("/test/file.txt")
	if err != nil {
		t.Errorf("GetHistory failed: %v", err)
	}
	if len(history) != 1 {
		t.Errorf("Expected 1 snapshot, got %d", len(history))
	}

	// 3. Snapshot changed buffer
	buf.Content = []byte("Version 2")
	time.Sleep(10 * time.Millisecond) // Ensure timestamp differs slightly
	err = vhm.SnapshotBuffer(buf)
	if err != nil {
		t.Errorf("SnapshotBuffer failed on update: %v", err)
	}

	// Verify 2 snapshots exist, newest first
	history, err = vhm.GetHistory("/test/file.txt")
	if err != nil {
		t.Errorf("GetHistory failed: %v", err)
	}
	if len(history) != 2 {
		t.Errorf("Expected 2 snapshots, got %d", len(history))
	}

	if string(history[0].Content) != "Version 2" {
		t.Errorf("Expected newest snapshot to be 'Version 2', got '%s'", history[0].Content)
	}
	if string(history[1].Content) != "Version 1" {
		t.Errorf("Expected oldest snapshot to be 'Version 1', got '%s'", history[1].Content)
	}
}
