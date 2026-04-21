package commands

import (
	"testing"
)

func TestCommandManager(t *testing.T) {
	cm := NewManager()

	executed := false
	cm.Register(&Command{
		ID:          "Test.Action",
		Description: "A test command",
		Execute: func(args map[string]interface{}) error {
			executed = true
			if val, ok := args["param"]; !ok || val != "value" {
				t.Errorf("Expected param=value, got %v", args)
			}
			return nil
		},
	})

	err := cm.Execute("Test.Action", map[string]interface{}{"param": "value"})
	if err != nil {
		t.Fatalf("Failed to execute command: %v", err)
	}

	if !executed {
		t.Error("Command function was not executed")
	}

	err = cm.Execute("NonExistent.Action", nil)
	if err == nil {
		t.Error("Expected error executing non-existent command, got nil")
	}
}
