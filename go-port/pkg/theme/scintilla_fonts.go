package theme

import (
	"encoding/json"
	"fmt"
)

// FontConfig represents the user's font configuration
type FontConfig struct {
	ProportionalFont string `json:"proportionalFont"`
	MonospaceFont    string `json:"monospaceFont"`
	MixFonts         bool   `json:"mixFonts"`
}

// IsProseText determines if the language type should use proportional fonts
func IsProseText(langType string) bool {
	switch langType {
	case "TXT", "L_TEXT", "L_USER", "MARKDOWN":
		return true
	default:
		return false
	}
}

// GetFontForLang returns the appropriate font name based on language type and configuration
func GetFontForLang(langType string, configJSON string) (string, error) {
	var config FontConfig
	if err := json.Unmarshal([]byte(configJSON), &config); err != nil {
		return "", fmt.Errorf("invalid config JSON: %w", err)
	}

	if config.MixFonts && IsProseText(langType) {
		return config.ProportionalFont, nil
	}
	return config.MonospaceFont, nil
}
