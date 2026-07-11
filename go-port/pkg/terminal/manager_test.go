package terminal

import (
	"testing"
	"time"
)

func TestTerminalManager(t *testing.T) {
	mgr := NewManager()

	// Test Spawn
	err := mgr.Spawn("term-1")
	if err != nil {
		t.Fatalf("Failed to spawn terminal: %v", err)
	}

	// Test Duplicate Spawn
	err = mgr.Spawn("term-1")
	if err == nil {
		t.Errorf("Expected error when spawning duplicate terminal id")
	}

	time.Sleep(10 * time.Millisecond)

	// Test Kill
	err = mgr.Kill("term-1")
	if err != nil {
		t.Errorf("Failed to kill terminal: %v", err)
	}

	// Test Kill Nonexistent
	err = mgr.Kill("term-99")
	if err == nil {
		t.Errorf("Expected error when killing nonexistent terminal")
	}
}
