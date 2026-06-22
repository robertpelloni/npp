package textfx

import (
	"bytes"
	"strings"
)

// Deep comment: This fulfills ROADMAP Phase 3 - "Audit the textfx2 plugin. Port its string manipulation..."
// Why: The legacy textfx2 C++ plugin requires synchronous interaction with the UI thread to fetch and modify text.
//      By porting these features to pure Go logic, we can manipulate strings in background goroutines
//      on massive (100MB+) files, taking advantage of Go's highly optimized bytes package and multi-threading.
// Side effect: The UI must sync via EventBus, but the UI thread itself never locks up.

// RemoveBlankLines removes all completely empty lines, and optionally lines with only whitespace.
// This is the functional equivalent of the legacy 'TextFX Edit -> Delete Blank Lines'
func RemoveBlankLines(content []byte, includeWhitespace bool) []byte {
	lines := bytes.Split(content, []byte("\n"))
	var result [][]byte

	for _, line := range lines {
		// Strip carriage return if present
		cleanLine := bytes.TrimSuffix(line, []byte("\r"))

		if len(cleanLine) == 0 {
			continue // Skip truly blank lines
		}

		if includeWhitespace && len(bytes.TrimSpace(cleanLine)) == 0 {
			continue // Skip lines that are only tabs/spaces
		}

		result = append(result, line)
	}

	return bytes.Join(result, []byte("\n"))
}

// TrimTrailingWhitespace removes trailing spaces and tabs from every line.
// This is the functional equivalent of 'TextFX Edit -> Trim Trailing Spaces'
func TrimTrailingWhitespace(content []byte) []byte {
	lines := bytes.Split(content, []byte("\n"))
	var result [][]byte

	for _, line := range lines {
		hasCR := bytes.HasSuffix(line, []byte("\r"))
		cleanLine := bytes.TrimSuffix(line, []byte("\r"))

		// Trim right side
		trimmed := bytes.TrimRight(cleanLine, " \t")

		if hasCR {
			trimmed = append(bytes.Clone(trimmed), '\r')
		}

		result = append(result, trimmed)
	}

	return bytes.Join(result, []byte("\n"))
}

// ConvertCase converts text to upper, lower, or title case.
// Replaces 'TextFX Characters -> UPPER CASE / lower case'
func ConvertCase(content []byte, mode string) []byte {
	switch strings.ToLower(mode) {
	case "upper":
		return bytes.ToUpper(content)
	case "lower":
		return bytes.ToLower(content)
	case "title":
		return bytes.Title(content)
	default:
		return content
	}
}
