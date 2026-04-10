package core

import "testing"

func TestBufferManager(t *testing.T) {
	bm := NewBufferManager()

	buf := bm.OpenBuffer("C:\\code\\main.go", "UTF-8")
	if buf == nil {
		t.Fatal("Expected buffer to be created")
	}

	active, err := bm.GetActiveBuffer()
	if err != nil {
		t.Fatalf("Error getting active buffer: %v", err)
	}

	if active.Filepath != "C:\\code\\main.go" {
		t.Errorf("Expected filepath C:\\code\\main.go, got %s", active.Filepath)
	}

	if active.IsDirty {
		t.Error("Buffer should not be dirty initially")
	}

	bm.MarkDirty(active.ID)
	if !active.IsDirty {
		t.Error("Buffer should be dirty after MarkDirty")
	}
}
