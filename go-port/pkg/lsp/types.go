package lsp

// LSP protocol types for textDocument/completion support.
// These follow the LSP 3.18 spec.

// TextDocumentIdentifier identifies a document by its URI.
type TextDocumentIdentifier struct {
	URI string `json:"uri"`
}

// Position in a text document (0-indexed line and column).
type Position struct {
	Line      int `json:"line"`
	Character int `json:"character"`
}

// CompletionContext provides context for a completion request.
type CompletionContext struct {
	TriggerKind      int    `json:"triggerKind"`
	TriggerCharacter string `json:"triggerCharacter,omitempty"`
}

// CompletionParams for the textDocument/completion request.
type CompletionParams struct {
	TextDocument TextDocumentIdentifier `json:"textDocument"`
	Position     Position               `json:"position"`
	Context      *CompletionContext     `json:"context,omitempty"`
}

// CompletionItem represents a single completion suggestion.
type CompletionItem struct {
	Label         string `json:"label"`
	Kind          int    `json:"kind,omitempty"`
	Detail        string `json:"detail,omitempty"`
	InsertText    string `json:"insertText,omitempty"`
	Documentation string `json:"documentation,omitempty"`
	FilterText    string `json:"filterText,omitempty"`
}

// CompletionList is a list of completion items returned by the server.
type CompletionList struct {
	IsIncomplete bool             `json:"isIncomplete"`
	Items        []CompletionItem `json:"items"`
}
