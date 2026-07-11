package lsp

import (
	"fmt"
	"strings"

	"github.com/notepad-plus-plus/ultra-project/pkg/core"
)

// Deep comment: This fulfills the final ROADMAP Phase 3 requirement:
// "Audit Geany features missing from Notepad++ (e.g., integrated VTE/Terminal, advanced symbol tree)."
// Why: We have already implemented the VTE/Terminal manager and the base Symbol extraction.
//      To achieve true Geany parity, we need a "Symbol Browser" controller that categorizes
//      extracted symbols (Functions, Variables, Classes) and provides a hierarchical data
//      structure for the Native UI (Qt/GTK) to render as a sidebar tree.

// SymbolNode represents a hierarchical element in the symbol tree
type SymbolNode struct {
	Name     string
	Kind     string // e.g., "Function", "Class", "Variable"
	Line     int
	Children []*SymbolNode
}

// SymbolBrowser acts as the controller for the Geany-style symbol sidebar
type SymbolBrowser struct {
	eb *core.EventBus
}

func NewSymbolBrowser(eb *core.EventBus) *SymbolBrowser {
	sb := &SymbolBrowser{
		eb: eb,
	}

	return sb
}

// CategorizeSymbols takes a flat list of extracted symbols and organizes them
// into a Geany-style hierarchy grouped by Kind.
func (sb *SymbolBrowser) CategorizeSymbols(rawSymbols []SymbolNode) map[string][]*SymbolNode {
	categorized := make(map[string][]*SymbolNode)

	for i := range rawSymbols {
		sym := &rawSymbols[i]

		// Normalize the kind
		kind := strings.Title(strings.ToLower(sym.Kind))
		if kind == "" {
			kind = "Unknown"
		}

		categorized[kind] = append(categorized[kind], sym)
	}

	return categorized
}

// PrintTree is a debug helper simulating the data export to the UI
func (sb *SymbolBrowser) PrintTree(categorized map[string][]*SymbolNode) {
	for kind, nodes := range categorized {
		fmt.Printf("[%s]\n", kind)
		for _, node := range nodes {
			fmt.Printf("  |- %s (Line %d)\n", node.Name, node.Line)
		}
	}
}
