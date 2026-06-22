package terminal

import (
	"fmt"
	"os/exec"
	"runtime"
	"sync"
)

// Deep comment: TerminalManager fulfills Phase 3 of ROADMAP.md - "Implement an integrated Terminal panel."
// Why: Modern IDEs (like Geany or VSCode) have an integrated shell. By routing a PTY through Go,
//      we can render the output in any frontend (Qt, GTK) without relying on platform-specific UI terminal widgets.
// Note: True PTY allocation is complex cross-platform. We start by wrapping standard OS command execution.

type Manager struct {
	mu       sync.RWMutex
	commands map[string]*exec.Cmd
}

func NewManager() *Manager {
	return &Manager{
		commands: make(map[string]*exec.Cmd),
	}
}

// Spawn starts a background terminal process (e.g., cmd.exe or bash)
func (m *Manager) Spawn(id string) error {
	m.mu.Lock()
	defer m.mu.Unlock()

	if _, exists := m.commands[id]; exists {
		return fmt.Errorf("terminal session %s already running", id)
	}

	var cmd *exec.Cmd
	if runtime.GOOS == "windows" {
		cmd = exec.Command("cmd.exe")
	} else {
		cmd = exec.Command("bash")
	}

	// For a real terminal, we would map Stdin/Stdout pipes here and stream them over EventBus
	// This is just the architectural scaffold

	err := cmd.Start()
	if err != nil {
		return fmt.Errorf("failed to start terminal: %w", err)
	}

	m.commands[id] = cmd
	return nil
}

// Kill terminates a running session
func (m *Manager) Kill(id string) error {
	m.mu.Lock()
	defer m.mu.Unlock()

	cmd, exists := m.commands[id]
	if !exists {
		return fmt.Errorf("terminal session %s not found", id)
	}

	if cmd.Process != nil {
		err := cmd.Process.Kill()
		go cmd.Process.Wait()
		if err != nil {
			return fmt.Errorf("failed to kill process: %w", err)
		}
	}

	delete(m.commands, id)
	return nil
}
