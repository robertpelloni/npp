package workspace

import (
	"testing"
	"github.com/notepad-plus-plus/ultra-project/pkg/core"
)

func TestWorkspaceLayout(t *testing.T) {
	layout := NewLayout()

	if layout.Placement != PlacementVertical {
		t.Errorf("Expected default placement to be VERTICAL")
	}

	layout.SetVerticalWidth(300)
	if layout.VerticalWidthPx != 300 {
		t.Errorf("Expected vertical width to be updated to 300")
	}

	layout.AddTab(core.BufferID("test1.txt"), "test1.txt")
	layout.AddTab(core.BufferID("test2.txt"), "test2.txt")

	if len(layout.Tabs) != 2 {
		t.Fatalf("Expected 2 tabs, got %d", len(layout.Tabs))
	}

	if layout.Tabs[0].IsActive {
		t.Errorf("Expected first tab to be deactivated")
	}

	if !layout.Tabs[1].IsActive {
		t.Errorf("Expected second tab to be active")
	}
}
