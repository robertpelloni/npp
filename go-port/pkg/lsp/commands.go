package lsp

import (
	"fmt"

	"github.com/notepad-plus-plus/ultra-project/pkg/commands"
	"github.com/notepad-plus-plus/ultra-project/pkg/core"
)

// RegisterLSPCommands binds LSP management actions to the global command router.
func RegisterLSPCommands(
	manager *commands.Manager,
	lspManager *Manager,
	bufManager *core.BufferManager,
) {
	manager.Register(&commands.Command{
		ID:          "LSP.Start",
		Description: "Start a language server for a specific language",
		Execute: func(args map[string]interface{}) error {
			lang, ok1 := args["language"].(string)
			cmd, ok2 := args["command"].(string)
			if !ok1 || !ok2 {
				return fmt.Errorf("missing language or command parameters")
			}

			// Optional args
			var cmdArgs []string
			if a, ok := args["args"].([]string); ok {
				cmdArgs = a
			}

			return lspManager.StartServer(lang, cmd, cmdArgs...)
		},
	})

	manager.Register(&commands.Command{
		ID:          "LSP.Completion",
		Description: "Request autocomplete suggestions",
		Execute: func(args map[string]interface{}) error {
			buf, err := bufManager.GetActiveBuffer()
			if err != nil {
				return err
			}

			line, _ := args["line"].(int)
			char, _ := args["character"].(int)

			suggestions, err := lspManager.RequestCompletion(buf, line, char)
			if err != nil {
				return err
			}

			fmt.Printf("[LSP] Suggestions: %v\n", suggestions)
			return nil
		},
	})
}
