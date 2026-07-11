package autosave

import (
	"log"

	"github.com/notepad-plus-plus/ultra-project/pkg/core"
)

// Deep comment: This file links Phase 2 of ROADMAP.md - "Hook into the Scintilla buffer modification events"
// Why: The legacy C++ UI throws SCN_SAVEPOINTREACHED and SCN_MODIFIED. We bridge those events via the EventBus
//      into the Go backend here, so that background versions are silently captured.

type EventListener struct {
	vhm      *VersionHistoryManager
	eventBus *core.EventBus
}

func NewEventListener(vhm *VersionHistoryManager, eb *core.EventBus) *EventListener {
	el := &EventListener{
		vhm:      vhm,
		eventBus: eb,
	}

	// Register to listen to buffer modifications
	el.eventBus.Subscribe("BufferChanged", el.handleBufferChanged)
	el.eventBus.Subscribe("BufferSaved", el.handleBufferSaved)

	return el
}

func (el *EventListener) handleBufferChanged(data interface{}) {
	// For high frequency edits, we might want to debounce this in the future.
	// For now, VersionHistoryManager.SnapshotBuffer prevents identical duplicates.
	buf, ok := data.(*core.Buffer)
	if !ok {
		log.Printf("[Autosave Hook] Invalid data type received on BufferChanged")
		return
	}

	err := el.vhm.SnapshotBuffer(buf)
	if err != nil {
		log.Printf("[Autosave Hook] Failed to snapshot buffer %s: %v", buf.ID, err)
	}
}

func (el *EventListener) handleBufferSaved(data interface{}) {
	// Similar handling logic for SCN_SAVEPOINTREACHED
	buf, ok := data.(*core.Buffer)
	if !ok {
		log.Printf("[Autosave Hook] Invalid data type received on BufferSaved")
		return
	}

	err := el.vhm.SnapshotBuffer(buf)
	if err != nil {
		log.Printf("[Autosave Hook] Failed to snapshot buffer %s on save: %v", buf.ID, err)
	}
}
