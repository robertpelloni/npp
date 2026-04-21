# Notepad++ Project Memory

This document tracks ongoing observations about the codebase, design preferences, and critical architectural constraints.

## Constraints & Anti-patterns
*   **Styler Inheritance:** Hardcoding font styles directly into `stylers.model.xml` is an anti-pattern that breaks Notepad++'s cascading style inheritance engine. Font styling MUST generally be left to the user via the Style Configurator UI.
*   **Version Control:** Every build needs a new version number updated across the project, ideally managed centrally via `VERSION.md` or `CHANGELOG.md`. Version bumps must be explicitly referenced in git commit messages.
*   **Commenting:** The user requires *deep, in-depth code commenting* for any touched code, detailing 'what', 'why', side effects, bugs, optimizations, and alternatives.

## Codebase Insights
*   **Scintilla:** The repository uses Scintilla for its core text editing components.
*   **UDLs:** Notepad++ handles custom languages (like Markdown) as User-Defined Languages (UDLs), internally represented by the `L_USER` language type enum.
*   **Language Type Access:** In the Notepad++ Scintilla component architecture, the current document's language type can be reliably accessed via the active buffer: `_pEditView->getCurrentBuffer()->getLangType()`.
*   **Testing:** The standard `make test` command is not configured for running tests in this repository. Use the `test_safe.sh` or Python testing scripts as appropriate.

## User Preferences
*   The UI must be fully skinnable, targeting a "super modern glass" aesthetic.
*   ClearType should be enforced.
