package theme

// Deep Comment: Scintilla Font Routing
// Why: Legacy Notepad++ sets a single global monospace font for all documents via 'stylers.model.xml'.
// This creates poor UX when writing prose (e.g., Markdown, TXT), where a proportional font (like Segoe UI or Arial) is standard.
// This file acts as a routing layer, dynamically mapping the current Scintilla LangType to a proportional or monospace font category.
// Optimization: We decouple this logic from the UI thread and CGO overhead by maintaining a simple, fast switch statement mapping.
// Side Effects: This overrides the 'stylers.xml' default font specifically for prose file types, enforcing the new Glass UI paradigm.

// LangType mirrors the Scintilla language types defined in the C++ backend
type LangType int

const (
	// Core text/prose types that should be proportional
	L_TEXT LangType = 0
	L_USER LangType = 57 // Represents User-Defined Languages (UDL), commonly used for Markdown

	// Standard code types (Monospace)
	L_PHP  LangType = 1
	L_C    LangType = 2
	L_CPP  LangType = 3
	L_CS   LangType = 4
	L_OBJC LangType = 5
	L_JAVA LangType = 6
	L_RC   LangType = 7
	L_HTML LangType = 8
	L_XML  LangType = 9
	// ... we assume all others not explicitly whitelisted as prose default to monospace
)

// FontCategory defines whether a font is proportional or monospace
type FontCategory int

const (
	FontCategoryMonospace FontCategory = iota
	FontCategoryProportional
)

// DetermineFontCategory evaluates a Scintilla LangType and returns whether it should use a proportional or monospace font.
func DetermineFontCategory(langType int) FontCategory {
	switch LangType(langType) {
	case L_TEXT, L_USER:
		// Prose, Markdown, and standard TXT files are far more readable with Proportional fonts.
		return FontCategoryProportional
	default:
		// Default to monospace for all programming languages and unrecognized types
		return FontCategoryMonospace
	}
}
