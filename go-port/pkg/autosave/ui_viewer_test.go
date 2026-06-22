package autosave

import (
	"database/sql"
	"os"
	"testing"
	"time"

	_ "github.com/mattn/go-sqlite3"
	"github.com/notepad-plus-plus/ultra-project/pkg/core"
)

func TestTimelineViewer(t *testing.T) {
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

	// Create test snapshots
	buf := &core.Buffer{
		ID:       "test1",
		Filepath: "/test/file.txt",
		Content:  []byte("Short version"),
	}
	vhm.SnapshotBuffer(buf)

	time.Sleep(10 * time.Millisecond)

	buf.Content = []byte("This is a much longer version of the text buffer designed to test the truncation logic in the timeline viewer. It should cut off at exactly fifty characters.")
	vhm.SnapshotBuffer(buf)

	tv := NewTimelineViewer(vhm)
	timeline, err := tv.GetTimelineForFile("/test/file.txt")
	if err != nil {
		t.Fatalf("GetTimelineForFile failed: %v", err)
	}

	if len(timeline) != 2 {
		t.Fatalf("Expected 2 timeline nodes, got %d", len(timeline))
	}

	// The newest snapshot should be first
	if len(timeline[0].Preview) > 53 { // 50 chars + "..."
		t.Errorf("Expected preview string to be truncated, got length %d", len(timeline[0].Preview))
	}

	if timeline[1].Preview != "Short version" {
		t.Errorf("Expected exact preview for short text, got '%s'", timeline[1].Preview)
	}
}
