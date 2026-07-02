package autosave

import (
	"log"

	"github.com/notepad-plus-plus/ultra-project/pkg/core"
)

// HookDBManager binds the EventBus to the DBManager.
// Why: Bridges Scintilla's SCN_MODIFIED and SCN_SAVEPOINTREACHED events to trigger background
//      SQLite snapshots without blocking the UI thread.
func HookDBManager(dbManager *DBManager, bus core.EventBus) {
	bus.Subscribe("BufferChanged", func(payload interface{}) {
		// Asynchronous snapshot handling
		go func() {
			if event, ok := payload.(map[string]interface{}); ok {
				if path, hasPath := event["filepath"].(string); hasPath {
					if content, hasContent := event["content"].([]byte); hasContent {
						// Clone bytes to avoid mutation/buffer corruption bugs
						contentClone := append([]byte(nil), content...)
						id, err := dbManager.SaveSnapshot(path, contentClone)
						if err != nil {
							log.Printf("[Autosave] Failed to save snapshot for %s: %v", path, err)
						} else {
							log.Printf("[Autosave] Snapshot triggered for %s (id: %s)", path, id)
						}
					}
				}
			}
		}()
	})

	bus.Subscribe("BufferSaved", func(payload interface{}) {
		// Asynchronous snapshot handling
		go func() {
			if event, ok := payload.(map[string]interface{}); ok {
				if path, hasPath := event["filepath"].(string); hasPath {
					if content, hasContent := event["content"].([]byte); hasContent {
						// Clone bytes to avoid mutation/buffer corruption bugs
						contentClone := append([]byte(nil), content...)
						id, err := dbManager.SaveSnapshot(path, contentClone)
						if err != nil {
							log.Printf("[Autosave] Failed to save snapshot for %s: %v", path, err)
						} else {
							log.Printf("[Autosave] Snapshot triggered for %s (id: %s)", path, id)
						}
					}
				}
			}
		}()
	})
}
