package markdown

import (
	"bytes"
	// To actually render markdown properly later, we'll pull in a dependency
	// like github.com/yuin/goldmark. For now, we mock the parsing layer.
)

// Deep comment: This establishes the core Markdown parser logic requested natively.
// Why: Legacy Notepad++ required external plugins to preview markdown. Building it
//      into the Go backend allows us to parse and stream HTML to the Native UI (e.g. Qt WebEngine).
// Side effects: Markdown rendering can be CPU intensive. The UI must request parses asynchronously.

type Parser struct{}

func NewParser() *Parser {
	return &Parser{}
}

// RenderToHTML takes raw markdown and returns a buffer of HTML.
func (p *Parser) RenderToHTML(md []byte) ([]byte, error) {
	// Stub logic: In reality, we'd use goldmark or similar to render this.
	// For testing, we just wrap the markdown in <pre> tags to show it was processed.

	var html bytes.Buffer
	html.WriteString("<html><body><pre>\n")
	html.Write(md)
	html.WriteString("\n</pre></body></html>")

	return html.Bytes(), nil
}
