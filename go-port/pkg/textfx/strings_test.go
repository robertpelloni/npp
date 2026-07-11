package textfx

import (
	"bytes"
	"testing"
)

func TestRemoveBlankLines(t *testing.T) {
	input := []byte("Line 1\n\nLine 2\n  \t  \nLine 3\r\n\r\nLine 4")

	// Test without stripping whitespace lines
	expectedNoWhitespaceStrip := []byte("Line 1\nLine 2\n  \t  \nLine 3\r\nLine 4")
	resultNoWs := RemoveBlankLines(input, false)
	if !bytes.Equal(resultNoWs, expectedNoWhitespaceStrip) {
		t.Errorf("RemoveBlankLines(false) failed.\nGot: %q\nExp: %q", resultNoWs, expectedNoWhitespaceStrip)
	}

	// Test with stripping whitespace lines
	expectedWithWhitespaceStrip := []byte("Line 1\nLine 2\nLine 3\r\nLine 4")
	resultWithWs := RemoveBlankLines(input, true)
	if !bytes.Equal(resultWithWs, expectedWithWhitespaceStrip) {
		t.Errorf("RemoveBlankLines(true) failed.\nGot: %q\nExp: %q", resultWithWs, expectedWithWhitespaceStrip)
	}
}

func TestTrimTrailingWhitespace(t *testing.T) {
	input := []byte("Line 1  \nLine 2\t\t\nLine 3 \t \r\nLine 4")
	expected := []byte("Line 1\nLine 2\nLine 3\r\nLine 4")

	result := TrimTrailingWhitespace(input)
	if !bytes.Equal(result, expected) {
		t.Errorf("TrimTrailingWhitespace failed.\nGot: %q\nExp: %q", result, expected)
	}
}

func TestConvertCase(t *testing.T) {
	input := []byte("hello WORLD")

	if res := ConvertCase(input, "upper"); string(res) != "HELLO WORLD" {
		t.Errorf("Upper case failed: got %s", res)
	}

	if res := ConvertCase(input, "lower"); string(res) != "hello world" {
		t.Errorf("Lower case failed: got %s", res)
	}

	if res := ConvertCase(input, "title"); string(res) != "Hello WORLD" {
		t.Errorf("Title case failed: got %s", res)
	}
}
