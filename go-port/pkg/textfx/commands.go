package textfx

import (
	"bytes"
	"fmt"

	"github.com/notepad-plus-plus/ultra-project/pkg/commands"
	"github.com/notepad-plus-plus/ultra-project/pkg/core"
)

// RegisterTextFXCommands registers the ported string manipulation features
// with the global CommandManager so they can be dispatched by the Native UI.
func RegisterTextFXCommands(cmdManager *commands.Manager, bufManager *core.BufferManager) {
	cmdManager.Register(&commands.Command{
		ID:          "TextFX.RemoveBlankLines",
		Description: "Removes all completely empty lines from the active buffer.",
		Execute: func(args map[string]interface{}) error {
			return applyTextFX(bufManager, false, func(content []byte) []byte {
				return RemoveBlankLines(content, false)
			})
		},
	})

	cmdManager.Register(&commands.Command{
		ID:          "TextFX.TrimTrailingWhitespace",
		Description: "Removes trailing spaces and tabs from every line.",
		Execute: func(args map[string]interface{}) error {
			return applyTextFX(bufManager, false, TrimTrailingWhitespace)
		},
	})

	cmdManager.Register(&commands.Command{
		ID:          "TextFX.ConvertCaseUpper",
		Description: "Converts the active buffer text to UPPER CASE.",
		Execute: func(args map[string]interface{}) error {
			return applyTextFX(bufManager, false, func(content []byte) []byte {
				return ConvertCase(content, "upper")
			})
		},
	})
}

// applyTextFX is a helper to fetch the active buffer, apply a text mutation, and apply the delta.
func applyTextFX(bufManager *core.BufferManager, includeWhitespace bool, fx func([]byte) []byte) error {
	buf, err := bufManager.GetActiveBuffer()
	if err != nil {
		return fmt.Errorf("no active buffer to format: %w", err)
	}

	newContent := fx(buf.Content)

	// Fast path check to avoid recording a delta if nothing changed
	if bytes.Equal(newContent, buf.Content) {
		return nil
	}

	// For a full-buffer formatting operation, the 'removed' text is the entire buffer,
	// and the 'inserted' text is the new formatted string starting at position 0.
	err = bufManager.ApplyEdit(buf.ID, 0, buf.Content, newContent)
	if err != nil {
		return fmt.Errorf("failed to apply textfx edit: %w", err)
	}

	return nil
}
