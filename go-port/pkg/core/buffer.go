package core

import (
	"fmt"
	"sync"
	"time"
)

// Deep comment: Buffer acts as the central data model for a single open file in the editor.
// Why: In the legacy C++ codebase, `Buffer.cpp` was heavily tied to Scintilla Document pointers
//      and Win32 file handles. Here in Go, we abstract it to represent pure state.
//      The UI layer (via CGO or native) will listen to changes in this struct to update Scintilla.
// Side effects: Keeping file content purely in Go memory might duplicate memory if Scintilla
//               also holds a copy. We will need to decide if Go acts as the single source of truth
//               and feeds Scintilla, or if Scintilla owns the text buffer and Go just manages metadata.

type BufferID string

type Buffer struct {
	ID           BufferID
	Filepath     string
	Content      []byte
	IsDirty      bool
	IsReadOnly   bool
	Encoding     string // e.g., "UTF-8", "ANSI"
	LanguageType string // Maps to L_USER, L_CPP, etc.
	LastModified time.Time

	undoStack [][]byte
	redoStack [][]byte
}

type BufferManager struct {
	mu       sync.RWMutex
	buffers  map[BufferID]*Buffer
	active   BufferID
	eventBus *EventBus
}

func NewBufferManager(eb *EventBus) *BufferManager {
	return &BufferManager{
		buffers:  make(map[BufferID]*Buffer),
		eventBus: eb,
	}
}

func (bm *BufferManager) OpenBuffer(filepath string, encoding string) *Buffer {
	bm.mu.Lock()
	defer bm.mu.Unlock()
	// In a real scenario, this would generate a UUID or hash
	id := BufferID(filepath)

	buf := &Buffer{
		ID:           id,
		Filepath:     filepath,
		IsDirty:      false,
		Encoding:     encoding,
		LanguageType: "TXT", // Default, would be inferred from extension
		LastModified: time.Now(),
	}

	bm.buffers[id] = buf
	bm.active = id

	if bm.eventBus != nil {
		bm.eventBus.Publish("BufferOpened", buf)
	}

	return buf
}

func (bm *BufferManager) GetActiveBuffer() (*Buffer, error) {
	bm.mu.RLock()
	defer bm.mu.RUnlock()

	if buf, exists := bm.buffers[bm.active]; exists {
		return buf, nil
	}
	return nil, fmt.Errorf("no active buffer")
}

func (bm *BufferManager) MarkDirty(id BufferID) {
	bm.mu.Lock()
	defer bm.mu.Unlock()

	if buf, exists := bm.buffers[id]; exists {
		// Save current state to undo stack before marking dirty if it's a new change
		// In a real editor, this would be more granular (diffs)
		buf.undoStack = append(buf.undoStack, append([]byte(nil), buf.Content...))
		buf.redoStack = nil // Clear redo stack on new change

		buf.IsDirty = true
		if bm.eventBus != nil {
			bm.eventBus.Publish("BufferChanged", buf)
		}
	}
}

func (bm *BufferManager) Undo(id BufferID) error {
	bm.mu.Lock()
	defer bm.mu.Unlock()

	buf, exists := bm.buffers[id]
	if !exists {
		return fmt.Errorf("buffer not found")
	}

	if len(buf.undoStack) == 0 {
		return fmt.Errorf("nothing to undo")
	}

	// Move current to redo
	buf.redoStack = append(buf.redoStack, append([]byte(nil), buf.Content...))

	// Pop from undo
	lastIdx := len(buf.undoStack) - 1
	buf.Content = buf.undoStack[lastIdx]
	buf.undoStack = buf.undoStack[:lastIdx]

	if bm.eventBus != nil {
		bm.eventBus.Publish("BufferChanged", buf)
	}
	return nil
}

func (bm *BufferManager) Redo(id BufferID) error {
	bm.mu.Lock()
	defer bm.mu.Unlock()

	buf, exists := bm.buffers[id]
	if !exists {
		return fmt.Errorf("buffer not found")
	}

	if len(buf.redoStack) == 0 {
		return fmt.Errorf("nothing to redo")
	}

	// Move current to undo
	buf.undoStack = append(buf.undoStack, append([]byte(nil), buf.Content...))

	// Pop from redo
	lastIdx := len(buf.redoStack) - 1
	buf.Content = buf.redoStack[lastIdx]
	buf.redoStack = buf.redoStack[:lastIdx]

	if bm.eventBus != nil {
		bm.eventBus.Publish("BufferChanged", buf)
	}
	return nil
}

func (bm *BufferManager) CloseBuffer(id BufferID) error {
	bm.mu.Lock()
	defer bm.mu.Unlock()

	buf, exists := bm.buffers[id]
	if !exists {
		return fmt.Errorf("buffer not found")
	}

	delete(bm.buffers, id)

	if bm.active == id {
		bm.active = ""
		// Pick another buffer to make active if available
		for nextID := range bm.buffers {
			bm.active = nextID
			break
		}
	}

	if bm.eventBus != nil {
		bm.eventBus.Publish("BufferClosed", buf)
	}

	return nil
}

func (bm *BufferManager) SwitchToBuffer(id BufferID) error {
	bm.mu.Lock()
	defer bm.mu.Unlock()

	if _, exists := bm.buffers[id]; !exists {
		return fmt.Errorf("buffer not found")
	}
	bm.active = id
	if bm.eventBus != nil {
		bm.eventBus.Publish("BufferActivated", id)
	}
	return nil
}
