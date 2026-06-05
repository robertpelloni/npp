package integration

import (
	"testing"

	"github.com/notepad-plus-plus/ultra-project/pkg/commands"
	"github.com/notepad-plus-plus/ultra-project/pkg/config"
	"github.com/notepad-plus-plus/ultra-project/pkg/core"
	"github.com/notepad-plus-plus/ultra-project/pkg/workspace"
)

func TestEndToEndFileWorkflow(t *testing.T) {
	// 1. Initialize System Components
	eventBus := core.NewEventBus()
	bufManager := core.NewBufferManager(eventBus)
	appConfig := config.DefaultConfig()
	layout := workspace.NewLayout()
	cmdManager := commands.NewManager()

	commands.RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)

	// 2. Subscribe to events for verification
	bufferOpened := false
	eventBus.Subscribe("BufferOpened", func(payload interface{}) {
		bufferOpened = true
	})

	// 3. Execute "File.New" command
	err := cmdManager.Execute("File.New", nil)
	if err != nil {
		t.Fatalf("File.New failed: %v", err)
	}

	// 4. Verify model states
	buf, err := bufManager.GetActiveBuffer()
	if err != nil {
		t.Fatalf("Failed to get active buffer: %v", err)
	}

	if buf.Filepath != "new_1.txt" {
		t.Errorf("Expected filepath new_1.txt, got %s", buf.Filepath)
	}

	if len(layout.Tabs) != 1 {
		t.Errorf("Expected 1 tab in layout, got %d", len(layout.Tabs))
	}

	if !bufferOpened {
		t.Error("Expected BufferOpened event to be published")
	}

	// 5. Execute "View.ToggleVerticalTabs" and verify config change
	initialPlacement := layout.Placement
	err = cmdManager.Execute("View.ToggleVerticalTabs", nil)
	if err != nil {
		t.Fatalf("View.ToggleVerticalTabs failed: %v", err)
	}

	if layout.Placement == initialPlacement {
		t.Errorf("Expected layout placement to toggle, but it stayed %s", layout.Placement)
	}

	// 6. Verify Command Mapping
	if name, ok := commands.CommandIDToName[41001]; !ok || name != "File.New" {
		t.Errorf("Expected mapping for 41001 to be File.New, got %s", name)
	}
}
