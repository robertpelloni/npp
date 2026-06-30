package theme

import "testing"

func TestGetFontForLang(t *testing.T) {
	configJSON := `{"proportionalFont": "Segoe UI", "monospaceFont": "Consolas", "mixFonts": true}`

	tests := []struct {
		langType string
		expected string
	}{
		{"L_TEXT", "Segoe UI"},
		{"MARKDOWN", "Segoe UI"},
		{"L_CPP", "Consolas"},
		{"L_GO", "Consolas"},
	}

	for _, test := range tests {
		result, err := GetFontForLang(test.langType, configJSON)
		if err != nil {
			t.Fatalf("unexpected error: %v", err)
		}
		if result != test.expected {
			t.Errorf("GetFontForLang(%q) = %q; want %q", test.langType, result, test.expected)
		}
	}

	// Test with MixFonts disabled
	configJSONNoMix := `{"proportionalFont": "Segoe UI", "monospaceFont": "Consolas", "mixFonts": false}`
	result, err := GetFontForLang("L_TEXT", configJSONNoMix)
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
	if result != "Consolas" {
		t.Errorf("GetFontForLang(\"L_TEXT\") with MixFonts=false = %q; want \"Consolas\"", result)
	}
}
