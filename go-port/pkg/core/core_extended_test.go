package core

import (
	"testing"
)

func TestSearchService(t *testing.T) {
	s := NewSearchService()
	content := []byte("hello world, hello golang")

	t.Run("Literal search", func(t *testing.T) {
		results := s.FindAll(content, "hello")
		if len(results) != 2 {
			t.Errorf("expected 2 results, got %d", len(results))
		}
		if results[0].Start != 0 || results[0].End != 5 {
			t.Errorf("unexpected first match: %+v", results[0])
		}
	})

	t.Run("Regex search", func(t *testing.T) {
		results, err := s.FindAllRegex(content, "h[a-z]llo")
		if err != nil {
			t.Fatalf("regex failed: %v", err)
		}
		if len(results) != 2 {
			t.Errorf("expected 2 results, got %d", len(results))
		}
	})
}

func TestBufferManagerExtended(t *testing.T) {
	eb := NewEventBus()
	bm := NewBufferManager(eb)

	b1 := bm.OpenBuffer("file1.txt", "UTF-8")
	b2 := bm.OpenBuffer("file2.txt", "UTF-8")

	if bm.active != b2.ID {
		t.Errorf("expected b2 to be active, got %s", bm.active)
	}

	t.Run("Switch buffer", func(t *testing.T) {
		err := bm.SwitchToBuffer(b1.ID)
		if err != nil {
			t.Fatalf("switch failed: %v", err)
		}
		if bm.active != b1.ID {
			t.Errorf("expected b1 to be active, got %s", bm.active)
		}
	})

	t.Run("Close buffer", func(t *testing.T) {
		err := bm.CloseBuffer(b1.ID)
		if err != nil {
			t.Fatalf("close failed: %v", err)
		}
		if _, exists := bm.buffers[b1.ID]; exists {
			t.Error("expected b1 to be removed from manager")
		}
		if bm.active != b2.ID {
			t.Errorf("expected b2 to become active, got %s", bm.active)
		}
	})
}
