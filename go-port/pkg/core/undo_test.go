package core

import "testing"

func TestUndoRedo(t *testing.T) {
	eb := NewEventBus()
	bm := NewBufferManager(eb)

	buf := bm.OpenBuffer("test.txt", "UTF-8")

	// Insert "Hello"
	if err := bm.ApplyEdit(buf.ID, 0, nil, []byte("Hello")); err != nil {
		t.Fatalf("ApplyEdit failed: %v", err)
	}

	// Insert " World"
	if err := bm.ApplyEdit(buf.ID, 5, nil, []byte(" World")); err != nil {
		t.Fatalf("ApplyEdit failed: %v", err)
	}

	expected := "Hello World"
	if string(buf.Content) != expected {
		t.Fatalf("expected content %q, got %q", expected, string(buf.Content))
	}

	// Undo should remove " World"
	if err := bm.Undo(buf.ID); err != nil {
		t.Fatalf("Undo failed: %v", err)
	}
	expected = "Hello"
	if string(buf.Content) != expected {
		t.Fatalf("after undo expected %q, got %q", expected, string(buf.Content))
	}

	// Redo should reapply " World"
	if err := bm.Redo(buf.ID); err != nil {
		t.Fatalf("Redo failed: %v", err)
	}
	expected = "Hello World"
	if string(buf.Content) != expected {
		t.Fatalf("after redo expected %q, got %q", expected, string(buf.Content))
	}

	// Delete " World"
	if err := bm.ApplyEdit(buf.ID, 5, []byte(" World"), nil); err != nil {
		t.Fatalf("delete ApplyEdit failed: %v", err)
	}
	expected = "Hello"
	if string(buf.Content) != expected {
		t.Fatalf("after delete expected %q, got %q", expected, string(buf.Content))
	}

	// Undo delete should restore " World"
	if err := bm.Undo(buf.ID); err != nil {
		t.Fatalf("Undo delete failed: %v", err)
	}
	expected = "Hello World"
	if string(buf.Content) != expected {
		t.Fatalf("after undo delete expected %q, got %q", expected, string(buf.Content))
	}

	// Redo delete should remove again
	if err := bm.Redo(buf.ID); err != nil {
		t.Fatalf("Redo delete failed: %v", err)
	}
	expected = "Hello"
	if string(buf.Content) != expected {
		t.Fatalf("after redo delete expected %q, got %q", expected, string(buf.Content))
	}
}
