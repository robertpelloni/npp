package ui

import (
	"fmt"
	"sync"
)

// Deep comment: This fulfills ROADMAP Phase 4 - "Implement a fully functional Vertical Tabs dockable pane"
// Why: Modern IDEs heavily rely on vertical sidebars for file management to save vertical screen space.
//      This Go package acts as the logical controller. The actual painting will occur in the Qt/GTK frontend.

type TabAction string

const (
	ActionClose      TabAction = "Close"
	ActionCloseAll   TabAction = "CloseAll"
	ActionCloseOther TabAction = "CloseOther"
	ActionPin        TabAction = "Pin"
)

// Tab represents a single file/buffer in the vertical tab pane
type Tab struct {
	ID       string
	Label    string
	IsPinned bool
	IsDirty  bool
}

// VerticalTabsManager acts as the controller for the Vertical Tabs UI panel
type VerticalTabsManager struct {
	mu   sync.RWMutex
	tabs []*Tab
}

func NewVerticalTabsManager() *VerticalTabsManager {
	return &VerticalTabsManager{
		tabs: make([]*Tab, 0),
	}
}

// AddTab registers a new buffer into the vertical tabs pane
func (vtm *VerticalTabsManager) AddTab(id string, label string) {
	vtm.mu.Lock()
	defer vtm.mu.Unlock()

	// Check if already exists
	for _, t := range vtm.tabs {
		if t.ID == id {
			return
		}
	}

	vtm.tabs = append(vtm.tabs, &Tab{
		ID:       id,
		Label:    label,
		IsPinned: false,
		IsDirty:  false,
	})
}

// PerformAction replicates the standard context menu actions of a horizontal tab bar
func (vtm *VerticalTabsManager) PerformAction(id string, action TabAction) error {
	vtm.mu.Lock()
	defer vtm.mu.Unlock()

	switch action {
	case ActionClose:
		vtm.removeTab(id)
	case ActionCloseAll:
		var pinned []*Tab
		for _, t := range vtm.tabs {
			if t.IsPinned {
				pinned = append(pinned, t)
			}
		}
		vtm.tabs = pinned
	case ActionCloseOther:
		var remaining []*Tab
		for _, t := range vtm.tabs {
			if t.IsPinned || t.ID == id {
				remaining = append(remaining, t)
			}
		}
		vtm.tabs = remaining
	case ActionPin:
		for _, t := range vtm.tabs {
			if t.ID == id {
				t.IsPinned = !t.IsPinned // Toggle pin state
				break
			}
		}
	default:
		return fmt.Errorf("unknown tab action: %s", action)
	}
	return nil
}

func (vtm *VerticalTabsManager) removeTab(id string) {
	for i, t := range vtm.tabs {
		if t.ID == id {
			// Fast slice deletion
			vtm.tabs = append(vtm.tabs[:i], vtm.tabs[i+1:]...)

			break
		}
	}
}

// GetTabs returns a read-only copy of the current tab state for UI rendering
func (vtm *VerticalTabsManager) GetTabs() []Tab {
	vtm.mu.RLock()
	defer vtm.mu.RUnlock()

	result := make([]Tab, len(vtm.tabs))
	for i, t := range vtm.tabs {
		result[i] = *t
	}
	return result
}
