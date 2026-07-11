package theme

import (
	"testing"
)

func TestDetermineFontCategory(t *testing.T) {
	tests := []struct {
		name     string
		langType int
		expected FontCategory
	}{
		{"Text/Prose should be proportional", int(L_TEXT), FontCategoryProportional},
		{"User Defined (Markdown) should be proportional", int(L_USER), FontCategoryProportional},
		{"C++ should be monospace", int(L_CPP), FontCategoryMonospace},
		{"HTML should be monospace", int(L_HTML), FontCategoryMonospace},
		{"Unknown should default to monospace", 999, FontCategoryMonospace},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := DetermineFontCategory(tt.langType); got != tt.expected {
				t.Errorf("DetermineFontCategory(%d) = %v, want %v", tt.langType, got, tt.expected)
			}
		})
	}
}

func TestEngine_GetContextualFontFamily(t *testing.T) {
	engine := NewEngine()

	// Using the default GlassDefault theme which has:
	// ProportionalFont: "Segoe UI"
	// MonospaceFont: "Consolas"

	tests := []struct {
		name     string
		langType int
		expected string
	}{
		{"Text prose", int(L_TEXT), "Segoe UI"},
		{"Markdown UDL", int(L_USER), "Segoe UI"},
		{"C language", int(L_C), "Consolas"},
		{"XML document", int(L_XML), "Consolas"},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := engine.GetContextualFontFamily(tt.langType); got != tt.expected {
				t.Errorf("GetContextualFontFamily(%d) = %v, want %v", tt.langType, got, tt.expected)
			}
		})
	}
}
