package commands

import (
	"fmt"
	"sync"

	"github.com/notepad-plus-plus/ultra-project/pkg/config"
	"github.com/notepad-plus-plus/ultra-project/pkg/core"
	"github.com/notepad-plus-plus/ultra-project/pkg/io"
	"github.com/notepad-plus-plus/ultra-project/pkg/workspace"
)

// searchCursor stores the current search state for FindNext/FindPrev navigation
type searchCursor struct {
	results []core.SearchResult
	index   int
}

var (
	searchState   = make(map[core.BufferID]*searchCursor)
	searchStateMu sync.Mutex
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

	var newFileCount int32
	var newFileMu sync.Mutex

	manager.Register(&Command{
		ID:          "File.New",
		Description: "Create a new empty buffer",
		Execute: func(args map[string]interface{}) error {
			newFileMu.Lock()
			newFileCount++
			count := newFileCount
			newFileMu.Unlock()

			title := fmt.Sprintf("new_%d.txt", count)
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
		Description: "Find text in the current buffer. Pass args[\"regex\"]=true for regex search.",
		Execute: func(args map[string]interface{}) error {
			query, ok := args["query"].(string)
			if !ok {
				return fmt.Errorf("missing query parameter")
			}

			buf, err := bufManager.GetActiveBuffer()
			if err != nil {
				return err
			}

			isRegex, _ := args["regex"].(bool)

			var results []core.SearchResult
			if isRegex {
				results, err = searchService.FindAllRegex(buf.Content, query)
				if err != nil {
					return fmt.Errorf("regex compile error: %w", err)
				}
			} else {
				results = searchService.FindAll(buf.Content, query)
			}

			fmt.Printf("Search found %d results for: %s (regex=%v)\n", len(results), query, isRegex)

			// Store search state for FindNext/FindPrev navigation
			searchStateMu.Lock()
			if len(results) > 0 {
				searchState[buf.ID] = &searchCursor{results: results, index: 0}
			} else {
				delete(searchState, buf.ID)
			}
			searchStateMu.Unlock()

			if bufManager.GetEventBus() != nil {
				bufManager.GetEventBus().Publish("Search.Results", results)
				if len(results) > 0 {
					bufManager.GetEventBus().Publish("Search.CurrentResult", results[0])
				}
			}
			return nil
		},
	})

	manager.Register(&Command{
		ID:          "Search.Replace",
		Description: "Replace text in the current buffer. Pass args[\"regex\"]=true for regex replace.",
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

			isRegex, _ := args["regex"].(bool)

			var results []core.SearchResult
			if isRegex {
				results, err = searchService.FindAllRegex(buf.Content, query)
				if err != nil {
					return fmt.Errorf("regex compile error: %w", err)
				}
				// For regex replace, use ReplaceAllRegex which supports capture groups
				newContent, err := searchService.ReplaceAllRegex(buf.Content, query, replacement)
				if err != nil {
					return fmt.Errorf("regex replace error: %w", err)
				}
				return bufManager.ApplyEdit(buf.ID, 0, buf.Content, newContent)
			}

			// Literal replace: find all occurrences and apply edits from end to start
			results = searchService.FindAll(buf.Content, query)
			if len(results) == 0 {
				return nil
			}

			replacementBytes := []byte(replacement)
			for i := len(results) - 1; i >= 0; i-- {
				result := results[i]
				removed := buf.Content[result.Start:result.End]
				err := bufManager.ApplyEdit(buf.ID, result.Start, removed, replacementBytes)
				if err != nil {
					return fmt.Errorf("replace failed at position %d: %w", result.Start, err)
				}
			}

			return nil
		},
	})

	manager.Register(&Command{
		ID:          "Search.FindNext",
		Description: "Jump to the next search result",
		Execute: func(args map[string]interface{}) error {
			buf, err := bufManager.GetActiveBuffer()
			if err != nil {
				return err
			}

			searchStateMu.Lock()
			cursor, exists := searchState[buf.ID]
			searchStateMu.Unlock()

			if !exists || len(cursor.results) == 0 {
				return fmt.Errorf("no search results to navigate")
			}

			cursor.index = (cursor.index + 1) % len(cursor.results)

			if bufManager.GetEventBus() != nil {
				bufManager.GetEventBus().Publish("Search.CurrentResult", cursor.results[cursor.index])
			}
			return nil
		},
	})

	manager.Register(&Command{
		ID:          "Search.FindPrev",
		Description: "Jump to the previous search result",
		Execute: func(args map[string]interface{}) error {
			buf, err := bufManager.GetActiveBuffer()
			if err != nil {
				return err
			}

			searchStateMu.Lock()
			cursor, exists := searchState[buf.ID]
			searchStateMu.Unlock()

			if !exists || len(cursor.results) == 0 {
				return fmt.Errorf("no search results to navigate")
			}

			cursor.index = (cursor.index - 1 + len(cursor.results)) % len(cursor.results)

			if bufManager.GetEventBus() != nil {
				bufManager.GetEventBus().Publish("Search.CurrentResult", cursor.results[cursor.index])
			}
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
			bufManager.MarkDirty(buf.ID)
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
			bufManager.MarkDirty(buf.ID)
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
			bufManager.MarkDirty(buf.ID)
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
