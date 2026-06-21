package theme

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"sync"
)

// Deep comment: Theme Engine handles the styling logic for the application.
// Why: The legacy 'stylers.model.xml' approach breaks native cascading styles.
//      This modern theme engine uses JSON configurations, allowing dynamic loading
//      and applying unified "Glass" styles across all UI layers (Gio, Qt, GTK).
// Optimization: We use RWMutex to ensure thread-safe concurrent reads from the UI.

// Config defines the visual parameters of a theme
type Config struct {
	Name            string `json:"name"`
	BackgroundColor string `json:"background_color"`
	ForegroundColor string `json:"foreground_color"`
	AccentColor     string `json:"accent_color"`
	FontFamily      string `json:"font_family"`
	FontSize        int    `json:"font_size"`
}

// Engine manages the active theme state
type Engine struct {
	mu           sync.RWMutex
	activeConfig Config
}

// NewEngine initializes a new theme engine with a default Glass theme
func NewEngine() *Engine {
	return &Engine{
		activeConfig: Config{
			Name:            "GlassDefault",
			BackgroundColor: "#1E1E1E",
			ForegroundColor: "#D4D4D4",
			AccentColor:     "#007ACC",
			FontFamily:      "Consolas",
			FontSize:        12,
		},
	}
}

// LoadTheme reads a JSON theme file and applies it
func (e *Engine) LoadTheme(filepath string) error {
	data, err := ioutil.ReadFile(filepath)
	if err != nil {
		return fmt.Errorf("failed to read theme file: %v", err)
	}

	var newConfig Config
	if err := json.Unmarshal(data, &newConfig); err != nil {
		return fmt.Errorf("failed to parse theme JSON: %v", err)
	}

	e.mu.Lock()
	defer e.mu.Unlock()
	e.activeConfig = newConfig

	// TODO: Dispatch a ThemeChanged event via EventBus
	return nil
}

// GetCurrentConfig returns a copy of the active theme configuration
func (e *Engine) GetCurrentConfig() Config {
	e.mu.RLock()
	defer e.mu.RUnlock()
	return e.activeConfig
}

// ApplyTheme applies the active theme directly (stub for Native UIs)
func (e *Engine) ApplyTheme() {
	e.mu.RLock()
	defer e.mu.RUnlock()
	// In reality, this would serialize the theme config over CGO to Qt/GTK
	fmt.Printf("[Theme Engine] Applying theme '%s': BG=%s, FG=%s\n",
		e.activeConfig.Name, e.activeConfig.BackgroundColor, e.activeConfig.ForegroundColor)
}
