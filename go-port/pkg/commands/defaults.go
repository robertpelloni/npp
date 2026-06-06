package commands

import (
	"fmt"

	"github.com/notepad-plus-plus/ultra-project/pkg/config"
	"github.com/notepad-plus-plus/ultra-project/pkg/core"
	"github.com/notepad-plus-plus/ultra-project/pkg/io"
	"github.com/notepad-plus-plus/ultra-project/pkg/workspace"
)

// Deep comment: RegisterDefaultCommands binds the application actions to the state models.
// Why: Instead of hardcoding logic inside the UI, the Go backend defines what a "Save" or "New Tab" does.
//      This is where we wire together the BufferManager, AppConfig, and Workspace models.

func RegisterDefaultCommands(
	manager *Manager,
	bufManager *core.BufferManager,
	appConfig *config.AppConfig,
	layout *workspace.Layout,
) {
	fileManager := io.NewFileManager()

	newFileCount := 0
	manager.Register(&Command{
		ID:          "File.New",
		Description: "Create a new empty buffer",
		Execute: func(args map[string]interface{}) error {
			newFileCount++
			title := fmt.Sprintf("new_%d.txt", newFileCount)
			buf := bufManager.OpenBuffer(title, "UTF-8")
			layout.AddTab(buf.ID, title)
			return nil
		},
	})

	manager.Register(&Command{
		ID:          "File.Open",
		Description: "Open an existing file",
		Execute: func(args map[string]interface{}) error {
			if filepath, ok := args["filepath"].(string); ok {
				content, err := fileManager.ReadFile(filepath)
				if err != nil {
					return err
				}
				buf := bufManager.OpenBuffer(filepath, "UTF-8")
				buf.Content = content
				layout.AddTab(buf.ID, filepath)
				return nil
			}
			return nil
		},
	})

	manager.Register(&Command{
		ID:          "File.Save",
		Description: "Save the current buffer",
		Execute: func(args map[string]interface{}) error {
			buf, err := bufManager.GetActiveBuffer()
			if err != nil {
				return err
			}

			err = fileManager.WriteFile(buf.Filepath, buf.Content)
			if err != nil {
				return err
			}

			buf.IsDirty = false
			return nil
		},
	})

	manager.Register(&Command{
		ID:          "File.CloseAll",
		Description: "Close all open buffers",
		Execute: func(args map[string]interface{}) error {
			// In a real implementation, this would clear the BufferManager and Layout
			return nil
		},
	})

	manager.Register(&Command{
		ID:          "Edit.Undo",
		Description: "Undo the last action",
		Execute: func(args map[string]interface{}) error {
			return nil
		},
	})

	manager.Register(&Command{
		ID:          "Edit.Redo",
		Description: "Redo the last undone action",
		Execute: func(args map[string]interface{}) error {
			return nil
		},
	})

	manager.Register(&Command{
		ID:          "Search.Find",
		Description: "Find text in the current buffer",
		Execute: func(args map[string]interface{}) error {
			return nil
		},
	})

	manager.Register(&Command{
		ID:          "Search.Replace",
		Description: "Replace text in the current buffer",
		Execute: func(args map[string]interface{}) error {
			return nil
		},
	})

	manager.Register(&Command{
		ID:          "View.ToggleVerticalTabs",
		Description: "Toggle vertical tab layout",
		Execute: func(args map[string]interface{}) error {
			appConfig.VerticalTabsEnabled = !appConfig.VerticalTabsEnabled
			if appConfig.VerticalTabsEnabled {
				layout.Placement = workspace.PlacementVertical
			} else {
				layout.Placement = workspace.PlacementTop
			}
			return nil
		},
	})

	manager.Register(&Command{
		ID:          "Settings.SetSkin",
		Description: "Change the UI skin theme",
		Execute: func(args map[string]interface{}) error {
			if skin, ok := args["skin"].(string); ok {
				appConfig.SkinTheme = skin
			}
			return nil
		},
	})
}
