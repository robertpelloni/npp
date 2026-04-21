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
	IsDirty      bool
	IsReadOnly   bool
	Encoding     string // e.g., "UTF-8", "ANSI"
	LanguageType string // Maps to L_USER, L_CPP, etc.
	LastModified time.Time
}

type BufferManager struct {
	buffers map[BufferID]*Buffer
	active  BufferID
}

func NewBufferManager() *BufferManager {
	return &BufferManager{
		buffers: make(map[BufferID]*Buffer),
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
	}
}
