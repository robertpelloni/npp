package commands

import (
	"fmt"
	"sync"
)

// Deep comment: CommandManager replaces the legacy NppBigSwitch.cpp.
// Why: Instead of a giant switch statement checking Windows Message IDs (WM_COMMAND),
//      we use a registry of named commands. The UI layers (Qt/GTK) simply call
//      Execute("File.Save") without needing to know the implementation details or IDs.
// Side effects: Slightly more overhead for string hashing on execution compared to an
//               integer switch, but heavily improves maintainability and cross-platform decoupling.

type CommandFunc func(args map[string]interface{}) error

type Command struct {
	ID          string
	Description string
	Execute     CommandFunc
}

type Manager struct {
	mu         sync.RWMutex
	commands   map[string]*Command
	middlewares []func(string, CommandFunc) CommandFunc
}

func NewManager() *Manager {
	return &Manager{
		commands: make(map[string]*Command),
	}
}


func (m *Manager) Use(middleware func(string, CommandFunc) CommandFunc) {
	m.mu.Lock()
	defer m.mu.Unlock()
	m.middlewares = append(m.middlewares, middleware)
}

func (m *Manager) Register(cmd *Command) {
	m.mu.Lock()
	defer m.mu.Unlock()
	m.commands[cmd.ID] = cmd
}

func (m *Manager) Execute(id string, args map[string]interface{}) error {
	m.mu.RLock()
	cmd, exists := m.commands[id]
	middlewares := m.middlewares
	m.mu.RUnlock()

	if !exists {
		return fmt.Errorf("command not found: %s", id)
	}

	execFunc := cmd.Execute
	// Apply middlewares in reverse order so the first one added is the outermost wrapper
	for i := len(middlewares) - 1; i >= 0; i-- {
		execFunc = middlewares[i](id, execFunc)
	}

	return execFunc(args)
}
