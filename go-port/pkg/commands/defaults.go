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
			buf, err := bufManager.GetActiveBuffer()
			for err == nil {
				_ = bufManager.CloseBuffer(buf.ID)
				buf, err = bufManager.GetActiveBuffer()
			}
			layout.Tabs = nil
			return nil
		},
	})

	manager.Register(&Command{
		ID:          "Edit.Undo",
		Description: "Undo the last action",
		Execute: func(args map[string]interface{}) error {
			buf, err := bufManager.GetActiveBuffer()
			if err != nil {
				return err
			}
			return bufManager.Undo(buf.ID)
		},
	})

	manager.Register(&Command{
		ID:          "Edit.Redo",
		Description: "Redo the last undone action",
		Execute: func(args map[string]interface{}) error {
			buf, err := bufManager.GetActiveBuffer()
			if err != nil {
				return err
			}
			return bufManager.Redo(buf.ID)
		},
	})

	searchService := core.NewSearchService()
	manager.Register(&Command{
		ID:          "Search.Find",
		Description: "Find text in the current buffer",
		Execute: func(args map[string]interface{}) error {
			query, ok := args["query"].(string)
			if !ok {
				return fmt.Errorf("missing query parameter")
			}

			buf, err := bufManager.GetActiveBuffer()
			if err != nil {
				return err
			}

			results := searchService.FindAll(buf.Content, query)
			fmt.Printf("Search found %d results for: %s\n", len(results), query)
			return nil
		},
	})

	manager.Register(&Command{
		ID:          "Search.Replace",
		Description: "Replace text in the current buffer",
		Execute: func(args map[string]interface{}) error {
			query, ok1 := args["query"].(string)
			replacement, ok2 := args["replacement"].(string)
			if !ok1 || !ok2 {
				return fmt.Errorf("missing parameters")
			}

			buf, err := bufManager.GetActiveBuffer()
			if err != nil {
				return err
			}

			buf.Content = searchService.ReplaceAll(buf.Content, query, replacement)
			bufManager.MarkDirty(buf.ID)
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
		ID:          "Format.ToUnix",
		Description: "Convert line endings to Unix (LF)",
		Execute: func(args map[string]interface{}) error {
			buf, err := bufManager.GetActiveBuffer()
			if err != nil {
				return err
			}
			return bufManager.ConvertLineEndings(buf.ID, "\n")
		},
	})

	manager.Register(&Command{
		ID:          "Format.ToDOS",
		Description: "Convert line endings to DOS (CRLF)",
		Execute: func(args map[string]interface{}) error {
			buf, err := bufManager.GetActiveBuffer()
			if err != nil {
				return err
			}
			return bufManager.ConvertLineEndings(buf.ID, "\r\n")
		},
	})

	manager.Register(&Command{
		ID:          "Format.ToMac",
		Description: "Convert line endings to Mac (CR)",
		Execute: func(args map[string]interface{}) error {
			buf, err := bufManager.GetActiveBuffer()
			if err != nil {
				return err
			}
			return bufManager.ConvertLineEndings(buf.ID, "\r")
		},
	})

	manager.Register(&Command{
		ID:          "Settings.Preferences",
		Description: "Show preference dialog",
		Execute: func(args map[string]interface{}) error {
			// Handled by UI wiring to toggle SettingsPanel visibility
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
