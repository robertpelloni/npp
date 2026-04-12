package markdown

import (
	"strings"
	"testing"
)

func TestMarkdownParser(t *testing.T) {
	parser := NewParser()

	md := []byte("# Hello\nThis is markdown.")
	html, err := parser.RenderToHTML(md)
	if err != nil {
		t.Fatalf("Failed to render markdown: %v", err)
	}

	out := string(html)
	if !strings.Contains(out, "<html>") || !strings.Contains(out, "# Hello") {
		t.Errorf("Expected markdown wrapped in HTML stub, got: %v", out)
	}
}
