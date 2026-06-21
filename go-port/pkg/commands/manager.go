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
	mu          sync.RWMutex
	commands    map[string]*Command
	compiledCmd map[string]CommandFunc
	middlewares []func(string, CommandFunc) CommandFunc
}

func NewManager() *Manager {
	return &Manager{
		commands:    make(map[string]*Command),
		compiledCmd: make(map[string]CommandFunc),
	}
}

func (m *Manager) Use(middleware func(string, CommandFunc) CommandFunc) {
	m.mu.Lock()
	defer m.mu.Unlock()
	m.middlewares = append(m.middlewares, middleware)
	// Recompile all commands to include the new middleware
	for id, cmd := range m.commands {
		m.compiledCmd[id] = m.compile(id, cmd.Execute)
	}
}

func (m *Manager) Register(cmd *Command) {
	m.mu.Lock()
	defer m.mu.Unlock()
	m.commands[cmd.ID] = cmd
	m.compiledCmd[cmd.ID] = m.compile(cmd.ID, cmd.Execute)
}

// compile applies all middlewares to a command function in reverse order,
// caching the result so it doesn't need to be rebuilt on every execution.
func (m *Manager) compile(id string, execute CommandFunc) CommandFunc {
	execFunc := execute
	for i := len(m.middlewares) - 1; i >= 0; i-- {
		execFunc = m.middlewares[i](id, execFunc)
	}
	return execFunc
}

func (m *Manager) Execute(id string, args map[string]interface{}) error {
	m.mu.RLock()
	compiledFunc, exists := m.compiledCmd[id]
	m.mu.RUnlock()

	if !exists {
		return fmt.Errorf("command not found: %s", id)
	}

	return compiledFunc(args)
}
