package theme

import (
	"os"
	"testing"
)

func TestEngine_DefaultTheme(t *testing.T) {
	engine := NewEngine()
	config := engine.GetCurrentConfig()

	if config.Name != "GlassDefault" {
		t.Errorf("Expected default theme name 'GlassDefault', got '%s'", config.Name)
	}
}

func TestEngine_LoadTheme(t *testing.T) {
	engine := NewEngine()

	// Create a temporary theme file
	tmpFile := "test_theme.json"
	themeData := `{
		"name": "CustomDark",
		"background_color": "#000000",
		"foreground_color": "#FFFFFF",
		"accent_color": "#FF0000",
		"font_family": "Arial",
		"font_size": 14
	}`
	err := os.WriteFile(tmpFile, []byte(themeData), 0644)
	if err != nil {
		t.Fatalf("Failed to create test theme file: %v", err)
	}
	defer os.Remove(tmpFile)

	// Load the theme
	err = engine.LoadTheme(tmpFile)
	if err != nil {
		t.Errorf("LoadTheme failed: %v", err)
	}

	// Verify loaded config
	config := engine.GetCurrentConfig()
	if config.Name != "CustomDark" {
		t.Errorf("Expected theme name 'CustomDark', got '%s'", config.Name)
	}
	if config.BackgroundColor != "#000000" {
		t.Errorf("Expected background color '#000000', got '%s'", config.BackgroundColor)
	}
}

func TestEngine_LoadInvalidTheme(t *testing.T) {
	engine := NewEngine()

	// Attempt to load a non-existent file
	err := engine.LoadTheme("non_existent_file.json")
	if err == nil {
		t.Error("Expected error when loading non-existent file, got nil")
	}

	// Create an invalid JSON file
	tmpFile := "invalid_theme.json"
	err = os.WriteFile(tmpFile, []byte(`{"name": "Broken", "font_size": "not_an_int"}`), 0644)
	if err != nil {
		t.Fatalf("Failed to create invalid test theme file: %v", err)
	}
	defer os.Remove(tmpFile)

	err = engine.LoadTheme(tmpFile)
	if err == nil {
		t.Error("Expected error when loading invalid JSON, got nil")
	}
}
