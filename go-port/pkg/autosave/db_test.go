package autosave

import (
	"os"
	"strings"
	"testing"
)

func TestDBManager(t *testing.T) {
	tempDir, err := os.MkdirTemp("", "npp_sqlite_test")
	if err != nil {
		t.Fatalf("Failed to create temp dir: %v", err)
	}
	defer os.RemoveAll(tempDir)

	manager, err := NewDBManager(tempDir)
	if err != nil {
		// go-sqlite3 returns a stub error when CGO_ENABLED=0
		// Skip the test gracefully instead of failing the suite
		if strings.Contains(err.Error(), "CGO_ENABLED=0") {
			t.Skipf("Skipping SQLite test: %v", err)
		}
		t.Fatalf("Failed to initialize SQLite manager: %v", err)
	}
	defer manager.Close()

	testFile := "C:\\code\\important.cpp"
	content1 := []byte("void main() {}")
	content2 := []byte("int main() { return 0; }")

	// Save snapshot 1
	id1, err := manager.SaveSnapshot(testFile, content1)
	if err != nil || id1 == "" {
		t.Fatalf("Failed to save snapshot 1 to SQLite: %v", err)
	}

	// Save snapshot 2
	id2, err := manager.SaveSnapshot(testFile, content2)
	if err != nil || id2 == "" {
		t.Fatalf("Failed to save snapshot 2 to SQLite: %v", err)
	}

	// Try deduplication
	idDup, err := manager.SaveSnapshot(testFile, content1)
	if err != nil {
		t.Fatalf("Failed deduplication save: %v", err)
	}
	if idDup[:13] != "deduplicated:" {
		t.Errorf("Expected deduplicated response string, got %s", idDup)
	}

	// Verify history count
	history, err := manager.GetHistory(testFile)
	if err != nil {
		t.Fatalf("Failed to retrieve SQLite history: %v", err)
	}

	if len(history) != 2 {
		t.Errorf("Expected 2 snapshots in history, got %d", len(history))
	}
}
