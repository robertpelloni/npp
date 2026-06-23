package lsp

import (
	"testing"

	"github.com/notepad-plus-plus/ultra-project/pkg/core"
)

func TestSymbolBrowser_CategorizeSymbols(t *testing.T) {
	eb := core.NewEventBus()
	sb := NewSymbolBrowser(eb)

	raw := []SymbolNode{
		{Name: "main", Kind: "FUNCTION", Line: 10},
		{Name: "AppConfig", Kind: "class", Line: 5},
		{Name: "helper", Kind: "function", Line: 20},
		{Name: "globalVar", Kind: "Variable", Line: 2},
		{Name: "badSymbol", Kind: "", Line: 99},
	}

	categorized := sb.CategorizeSymbols(raw)

	// Verify categories exist
	if len(categorized["Function"]) != 2 {
		t.Errorf("Expected 2 Functions, got %d", len(categorized["Function"]))
	}
	if len(categorized["Class"]) != 1 {
		t.Errorf("Expected 1 Class, got %d", len(categorized["Class"]))
	}
	if len(categorized["Variable"]) != 1 {
		t.Errorf("Expected 1 Variable, got %d", len(categorized["Variable"]))
	}
	if len(categorized["Unknown"]) != 1 {
		t.Errorf("Expected 1 Unknown, got %d", len(categorized["Unknown"]))
	}

	// Verify specific routing
	if categorized["Function"][0].Name != "main" {
		t.Errorf("Expected 'main' to be first function")
	}
}
