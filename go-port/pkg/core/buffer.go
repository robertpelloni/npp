package core

import (
	"fmt"
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
}

type BufferManager struct {
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
	if buf, exists := bm.buffers[bm.active]; exists {
		return buf, nil
	}
	return nil, fmt.Errorf("no active buffer")
}

func (bm *BufferManager) MarkDirty(id BufferID) {
	if buf, exists := bm.buffers[id]; exists {
		buf.IsDirty = true
		if bm.eventBus != nil {
			bm.eventBus.Publish("BufferChanged", buf)
		}
	}
}

func (bm *BufferManager) CloseBuffer(id BufferID) error {
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
	if _, exists := bm.buffers[id]; !exists {
		return fmt.Errorf("buffer not found")
	}
	bm.active = id
	if bm.eventBus != nil {
		bm.eventBus.Publish("BufferActivated", id)
	}
	return nil
}
