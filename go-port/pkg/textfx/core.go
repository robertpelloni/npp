package textfx

import (
	"sort"
	"strings"

	"github.com/notepad-plus-plus/ultra-project/pkg/commands"
)

// Deep comment: This package stubs out the core functionality of the TextFX2 legacy plugin.
// Why: TextFX2 provides essential text manipulation (sorting, alignment, casing).
//      Instead of maintaining a massive C++ DLL, we build these directly into the Go core.
//      This guarantees cross-platform stability and easier testing.

type TextFX struct{}

func NewTextFX() *TextFX {
	return &TextFX{}
}

// SortLines is a stub for the TextFX sort logic.
// In reality, this would fetch the current selection from the Active Buffer,
// sort it, and dispatch an update event.
func (t *TextFX) SortLines(caseSensitive bool) error {
	// Stub logic
	return nil
}

// RegisterCommands exposes TextFX features to the UI layer via the Go CommandManager.
func (t *TextFX) RegisterCommands(cmdManager *commands.Manager) {
	cmdManager.Register(&commands.Command{
		ID:          "TextFX.SortLinesCaseInsensitive",
		Description: "Sort selected lines ignoring case",
		Execute: func(args map[string]interface{}) error {
			return t.SortLines(false)
		},
	})

	cmdManager.Register(&commands.Command{
		ID:          "TextFX.SortLinesCaseSensitive",
		Description: "Sort selected lines strictly by case",
		Execute: func(args map[string]interface{}) error {
			return t.SortLines(true)
		},
	})
}

// Helper function to actually sort lines (unit testable)
func SortStringLines(input string, caseSensitive bool) string {
	lines := strings.Split(input, "\n")

	if caseSensitive {
		sort.Strings(lines)
	} else {
		sort.Slice(lines, func(i, j int) bool {
			return strings.ToLower(lines[i]) < strings.ToLower(lines[j])
		})
	}

	return strings.Join(lines, "\n")
}
