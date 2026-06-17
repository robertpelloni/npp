package config

import (
	"encoding/xml"
	"os"
)

// Deep comment: The Parameters/Config package manages the application's user preferences.
// Why: Replacing the tight coupling of C++ `Parameters.h` with a pure Go struct.
//      This struct acts as the single source of truth for UI features and editor behavior.
//      It specifically implements the new user requirements (Vertical tabs, typography, etc.)
// Side effects: Settings need to be serialized/deserialized (JSON/XML) to disk.

type AppConfig struct {
	XMLName xml.Name `xml:"AppConfig"`

	// UI Aesthetics
	SkinTheme       string `xml:"SkinTheme"`
	EnableClearType bool   `xml:"EnableClearType"`

	// Tab Management
	VerticalTabsEnabled bool `xml:"VerticalTabsEnabled"`
	VerticalTabWidth    int  `xml:"VerticalTabWidth"`

	// Typography
	MixFonts bool `xml:"MixFonts"` // Regular for standard text, monospace for code

	// Editor Behavior
	DisableAutocompleteInText bool `xml:"DisableAutocompleteInText"`
	AutosaveEnabled           bool `xml:"AutosaveEnabled"`
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

// SaveXML serializes the configuration to an XML file.
func (c *AppConfig) SaveXML(path string) error {
	f, err := os.Create(path)
	if err != nil {
		return err
	}
	defer f.Close()

	enc := xml.NewEncoder(f)
	enc.Indent("", "  ")
	return enc.Encode(c)
}

// LoadXML deserializes the configuration from an XML file.
func (c *AppConfig) LoadXML(path string) error {
	f, err := os.Open(path)
	if err != nil {
		return err
	}
	defer f.Close()

	dec := xml.NewDecoder(f)
	return dec.Decode(c)
}
