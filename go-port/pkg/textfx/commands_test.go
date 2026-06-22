package textfx

import (
	"bytes"
	"testing"

	"github.com/notepad-plus-plus/ultra-project/pkg/commands"
	"github.com/notepad-plus-plus/ultra-project/pkg/core"
)

func TestTextFXCommands(t *testing.T) {
	eb := core.NewEventBus()
	bm := core.NewBufferManager(eb)
	cm := commands.NewManager()

	RegisterTextFXCommands(cm, bm)

	// Create a test buffer
	buf := bm.OpenBuffer("test.txt", "UTF-8")

	// Test ConvertCaseUpper
	buf.Content = []byte("hello world")
	err := cm.Execute("TextFX.ConvertCaseUpper", nil)
	if err != nil {
		t.Fatalf("Command execution failed: %v", err)
	}
	if string(buf.Content) != "HELLO WORLD" {
		t.Errorf("Expected HELLO WORLD, got %s", buf.Content)
	}

	// Test TrimTrailingWhitespace
	buf.Content = []byte("line1  \nline2")
	err = cm.Execute("TextFX.TrimTrailingWhitespace", nil)
	if err != nil {
		t.Fatalf("Command execution failed: %v", err)
	}
	if string(buf.Content) != "line1\nline2" {
		t.Errorf("Expected trimmed content, got %q", buf.Content)
	}

	// Ensure undo delta was recorded
	err = bm.Undo(buf.ID)
	if err != nil {
		t.Fatalf("Undo failed: %v", err)
	}
	if string(buf.Content) != "line1  \nline2" {
		t.Errorf("Expected undo to restore trailing spaces, got %q", buf.Content)
	}
}

func TestApplyTextFXNoActiveBuffer(t *testing.T) {
	eb := core.NewEventBus()
	bm := core.NewBufferManager(eb)
	cm := commands.NewManager()

	RegisterTextFXCommands(cm, bm)

	// No buffer open
	err := cm.Execute("TextFX.ConvertCaseUpper", nil)
	if err == nil {
		t.Error("Expected error when executing command without active buffer, got nil")
	}
}

func TestTrimTrailingWhitespace_NoMutation(t *testing.T) {
	input := []byte("Line 1  \nLine 2")
	TrimTrailingWhitespace(input)

	// Ensure the original input byte slice wasn't mutated in place
	if !bytes.Equal(input, []byte("Line 1  \nLine 2")) {
		t.Errorf("TrimTrailingWhitespace mutated input array: %q", input)
	}
}
