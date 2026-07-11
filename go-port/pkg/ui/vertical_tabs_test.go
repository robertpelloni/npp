package ui

import (
	"testing"
)

func TestVerticalTabsManager(t *testing.T) {
	mgr := NewVerticalTabsManager()

	// Setup
	mgr.AddTab("buf1", "main.go")
	mgr.AddTab("buf2", "utils.go")
	mgr.AddTab("buf3", "config.json")

	// Test Pinning
	mgr.PerformAction("buf2", ActionPin)

	tabs := mgr.GetTabs()
	if !getTab(tabs, "buf2").IsPinned {
		t.Errorf("Expected buf2 to be pinned")
	}

	// Test CloseOther (should keep buf2 because it's pinned, and buf1 because it's the target)
	mgr.PerformAction("buf1", ActionCloseOther)
	tabs = mgr.GetTabs()

	if len(tabs) != 2 {
		t.Fatalf("Expected 2 tabs remaining, got %d", len(tabs))
	}
	if getTab(tabs, "buf3") != nil {
		t.Errorf("Expected buf3 to be closed")
	}

	// Test CloseAll (should keep buf2 because it's pinned)
	mgr.PerformAction("", ActionCloseAll)
	tabs = mgr.GetTabs()

	if len(tabs) != 1 || tabs[0].ID != "buf2" {
		t.Errorf("Expected only buf2 to remain after CloseAll, got %v", tabs)
	}

	// Test Close
	mgr.PerformAction("buf2", ActionClose)
	tabs = mgr.GetTabs()
	if len(tabs) != 0 {
		t.Errorf("Expected all tabs to be closed, got %v", tabs)
	}
}

func getTab(tabs []Tab, id string) *Tab {
	for _, t := range tabs {
		if t.ID == id {
			return &t
		}
	}
	return nil
}
