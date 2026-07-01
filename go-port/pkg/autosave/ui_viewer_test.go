package autosave

import (
	"testing"
)

func TestSafePreview(t *testing.T) {
	tests := []struct {
		input    string
		maxLen   int
		expected string
	}{
		{"Hello, World!", 5, "Hello..."},
		{"Hello", 10, "Hello"},
		{"你好世界", 2, "你好..."}, // Multi-byte UTF-8 test
		{"", 5, ""},
	}

	for _, test := range tests {
		result := safePreview([]byte(test.input), test.maxLen)
		if result != test.expected {
			t.Errorf("safePreview(%q, %d) = %q; want %q", test.input, test.maxLen, result, test.expected)
		}
	}
}
