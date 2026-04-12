package commands

import (
	"github.com/notepad-plus-plus/ultra-project/pkg/config"
	"github.com/notepad-plus-plus/ultra-project/pkg/core"
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

	manager.Register(&Command{
		ID:          "File.New",
		Description: "Create a new empty buffer",
		Execute: func(args map[string]interface{}) error {
			// Stub: In reality, we'd generate a unique ID
			title := "new_1.txt"
			buf := bufManager.OpenBuffer(title, "UTF-8")
			layout.AddTab(buf.ID, title)
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
