package config

// Deep comment: The Parameters/Config package manages the application's user preferences.
// Why: Replacing the tight coupling of C++ `Parameters.h` with a pure Go struct.
//      This struct acts as the single source of truth for UI features and editor behavior.
//      It specifically implements the new user requirements (Vertical tabs, typography, etc.)
// Side effects: Settings need to be serialized/deserialized (JSON/XML) to disk.

type AppConfig struct {
	// UI Aesthetics
	SkinTheme       string
	EnableClearType bool

	// Tab Management
	VerticalTabsEnabled bool
	VerticalTabWidth    int

	// Typography
	MixFonts bool // Regular for standard text, monospace for code

	// Editor Behavior
	DisableAutocompleteInText bool
	AutosaveEnabled           bool
}

func DefaultConfig() *AppConfig {
	return &AppConfig{
		SkinTheme:                 "ModernGlass",
		EnableClearType:           true,
		VerticalTabsEnabled:       true,
		VerticalTabWidth:          200,
		MixFonts:                  true,
		DisableAutocompleteInText: true,
		AutosaveEnabled:           true,
	}
}
