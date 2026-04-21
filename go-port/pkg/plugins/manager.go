package plugins

import (
	"fmt"
	"log"
	"sync"
)

// Deep comment: PluginManager establishes the interface for how the Go backend loads and interacts with plugins.
// Why: Legacy Notepad++ plugins are DLLs loaded via Win32 LoadLibrary. The ultra-project aims to support
//      multiple platforms. While we can use cgo to load C-style shared objects (.so, .dll, .dylib),
//      we need a generic Go interface that plugins must adhere to, abstracting OS-specific loading mechanics.
//      Submodules like TextFX2 will eventually be wrapped to conform to this interface.

// Plugin defines the lifecycle and capabilities of a generic Ultra-Project plugin.
type Plugin interface {
	Name() string
	Version() string
	Init() error
	Shutdown() error
	// Future enhancements: RegisterCommands(cmdManager), RegisterUIHooks(uiBridge), etc.
}

type Manager struct {
	mu      sync.RWMutex
	plugins map[string]Plugin
}

func NewManager() *Manager {
	return &Manager{
		plugins: make(map[string]Plugin),
	}
}

// LoadPlugin registers and initializes a plugin.
func (m *Manager) LoadPlugin(p Plugin) error {
	m.mu.Lock()
	defer m.mu.Unlock()

	name := p.Name()
	if _, exists := m.plugins[name]; exists {
		return fmt.Errorf("plugin %s is already loaded", name)
	}

	log.Printf("Initializing plugin: %s (v%s)\n", name, p.Version())
	if err := p.Init(); err != nil {
		return fmt.Errorf("failed to initialize plugin %s: %w", name, err)
	}

	m.plugins[name] = p
	return nil
}

// UnloadPlugin safely shuts down and removes a plugin.
func (m *Manager) UnloadPlugin(name string) error {
	m.mu.Lock()
	defer m.mu.Unlock()

	p, exists := m.plugins[name]
	if !exists {
		return fmt.Errorf("plugin %s is not loaded", name)
	}

	log.Printf("Shutting down plugin: %s\n", name)
	if err := p.Shutdown(); err != nil {
		return fmt.Errorf("failed to shutdown plugin %s cleanly: %w", name, err)
	}

	delete(m.plugins, name)
	return nil
}
