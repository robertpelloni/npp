package config

import "testing"

func TestDefaultConfig(t *testing.T) {
	cfg := DefaultConfig()

	if !cfg.VerticalTabsEnabled {
		t.Error("Expected vertical tabs to be enabled by default")
	}

	if cfg.VerticalTabWidth <= 0 {
		t.Error("Expected a positive vertical tab width")
	}

	if !cfg.MixFonts {
		t.Error("Expected font mixing to be enabled by default")
	}
}
