package core

import (
	"fmt"
	"regexp"
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

// EditDelta represents a single edit operation for delta-based undo/redo
// This replaces the O(n) full-buffer snapshot approach with O(k) where k is edit size
type EditDelta struct {
	// Position where the edit occurred (byte offset)
	Position int
	// Text that was removed (empty for pure insertions)
	Removed []byte
	// Text that was inserted (empty for pure deletions)
	Inserted []byte
}

// EditorUpdater defines the interface for notifying a native editor about buffer content changes.
// This decouples the core buffer management from the CGO/Scintilla bridge.
type EditorUpdater interface {
	// SetText replaces the entire editor content for the given buffer.
	SetText(bufID BufferID, content []byte)
	// InsertText inserts text at a given position in the editor.
	InsertText(bufID BufferID, position int, text []byte)
}

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

	// Delta-based undo/redo stacks - O(k) memory per operation instead of O(n)
	undoStack []*EditDelta
	redoStack []*EditDelta
}

type BufferManager struct {
	mu       sync.RWMutex
	buffers  map[BufferID]*Buffer
	active   BufferID
	eventBus *EventBus
	editor   EditorUpdater
}

func NewBufferManager(eb *EventBus) *BufferManager {
	return &BufferManager{
		buffers:  make(map[BufferID]*Buffer),
		eventBus: eb,
	}
}

// SetEditorUpdater sets the bridge to a native editor, enabling buffer-to-editor sync.
func (bm *BufferManager) SetEditorUpdater(eu EditorUpdater) {
	bm.mu.Lock()
	defer bm.mu.Unlock()
	bm.editor = eu
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

	// Notify the native editor about the new buffer content
	if bm.editor != nil {
		bm.editor.SetText(id, buf.Content)
	}

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

// RecordDelta records an edit operation for undo/redo
// This should be called BEFORE the edit is applied to Content
func (buf *Buffer) RecordDelta(position int, removed, inserted []byte) {
	// Save delta: we're about to replace `removed` with `inserted` at `position`
	buf.undoStack = append(buf.undoStack, &EditDelta{
		Position: position,
		Removed:  append([]byte(nil), removed...),
		Inserted: append([]byte(nil), inserted...),
	})
	// Clear redo stack on new edit
	buf.redoStack = nil
}

func (bm *BufferManager) MarkDirty(id BufferID) {
	bm.mu.Lock()
	defer bm.mu.Unlock()

	if buf, exists := bm.buffers[id]; exists {
		// Note: MarkDirty is now called AFTER the edit, so we need different logic
		// For now, we'll keep this as a marker but actual deltas are recorded by ApplyEdit
		buf.IsDirty = true
		if bm.eventBus != nil {
			bm.eventBus.Publish("BufferChanged", buf)
		}
	}
}

// ApplyEdit applies an edit and records it for undo/redo
// This is the primary method for modifying buffer content with delta tracking
func (bm *BufferManager) ApplyEdit(id BufferID, position int, removed, inserted []byte) error {
	bm.mu.Lock()
	defer bm.mu.Unlock()

	buf, exists := bm.buffers[id]
	if !exists {
		return fmt.Errorf("buffer not found")
	}

	// Record the delta BEFORE applying the edit
	buf.RecordDelta(position, removed, inserted)

	// Apply the edit: remove `removed` bytes at position, insert `inserted` bytes
	// Validate position
	if position < 0 || position > len(buf.Content) {
		return fmt.Errorf("invalid position: %d (content length: %d)", position, len(buf.Content))
	}

	// Calculate the actual end of removed text
	removeEnd := position + len(removed)
	if removeEnd > len(buf.Content) {
		removeEnd = len(buf.Content)
	}

	// Build new content: before + inserted + after
	newContent := make([]byte, 0, len(buf.Content)-len(removed)+len(inserted))
	newContent = append(newContent, buf.Content[:position]...)
	newContent = append(newContent, inserted...)
	if removeEnd < len(buf.Content) {
		newContent = append(newContent, buf.Content[removeEnd:]...)
	}

	buf.Content = newContent
	buf.IsDirty = true

	// Notify the native editor about the content update
	if bm.editor != nil {
		bm.editor.SetText(id, buf.Content)
	}

	if bm.eventBus != nil {
		bm.eventBus.Publish("BufferChanged", buf)
	}

	return nil
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

	// Pop the last delta
	lastIdx := len(buf.undoStack) - 1
	delta := buf.undoStack[lastIdx]
	buf.undoStack = buf.undoStack[:lastIdx]

	// Reverse the delta: remove inserted, re-insert removed
	// First, validate position
	if delta.Position < 0 || delta.Position > len(buf.Content) {
		return fmt.Errorf("undo failed: invalid position %d (content length: %d)", delta.Position, len(buf.Content))
	}

	// The inserted text is at delta.Position with length len(delta.Inserted)
	// We need to remove it and put back delta.Removed
	insertEnd := delta.Position + len(delta.Inserted)
	if insertEnd > len(buf.Content) {
		return fmt.Errorf("undo failed: inserted text extends beyond content")
	}

	// Build new content: before + removed + after
	newContent := make([]byte, 0, len(buf.Content)-len(delta.Inserted)+len(delta.Removed))
	newContent = append(newContent, buf.Content[:delta.Position]...)
	newContent = append(newContent, delta.Removed...)
	if insertEnd < len(buf.Content) {
		newContent = append(newContent, buf.Content[insertEnd:]...)
	}

	buf.Content = newContent

	// Push the SAME delta to redo stack
	// Redo will forward-apply this same delta (remove Removed, re-insert Inserted)
	buf.redoStack = append(buf.redoStack, delta)

	// Notify the native editor about the content update
	if bm.editor != nil {
		bm.editor.SetText(id, buf.Content)
	}

	if bm.eventBus != nil {
		bm.eventBus.Publish("BufferChanged", buf)
	}

	return nil
}

func (bm *BufferManager) GetEventBus() *EventBus {
	return bm.eventBus
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

	// Pop the last redo delta
	lastIdx := len(buf.redoStack) - 1
	delta := buf.redoStack[lastIdx]
	buf.redoStack = buf.redoStack[:lastIdx]

	// Apply the redo: this is the same as ApplyEdit but for redo stack
	if delta.Position < 0 || delta.Position > len(buf.Content) {
		return fmt.Errorf("redo failed: invalid position %d (content length: %d)", delta.Position, len(buf.Content))
	}

	removeEnd := delta.Position + len(delta.Removed)
	if removeEnd > len(buf.Content) {
		return fmt.Errorf("redo failed: removed text extends beyond content")
	}

	// Build new content: before + inserted + after
	newContent := make([]byte, 0, len(buf.Content)-len(delta.Removed)+len(delta.Inserted))
	newContent = append(newContent, buf.Content[:delta.Position]...)
	newContent = append(newContent, delta.Inserted...)
	if removeEnd < len(buf.Content) {
		newContent = append(newContent, buf.Content[removeEnd:]...)
	}

	buf.Content = newContent

	// Push the SAME delta back to undo stack
	// This delta represents the forward edit (remove Removed, insert Inserted)
	buf.undoStack = append(buf.undoStack, delta)

	// Notify the native editor about the content update
	if bm.editor != nil {
		bm.editor.SetText(id, buf.Content)
	}

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

func (bm *BufferManager) ConvertLineEndings(id BufferID, eol string) error {
	bm.mu.Lock()
	defer bm.mu.Unlock()

	buf, exists := bm.buffers[id]
	if !exists {
		return fmt.Errorf("buffer not found")
	}

	// Simple conversion logic
	content := string(buf.Content)
	content = regexp.MustCompile(`\r\n|\r|\n`).ReplaceAllString(content, eol)
	buf.Content = []byte(content)
	buf.IsDirty = true

	if bm.eventBus != nil {
		bm.eventBus.Publish("BufferChanged", buf)
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
