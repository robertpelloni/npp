# Notepad++ Ultra-Project Memory & Architecture Summary

## 1. Project Vision & Ultimate Goals
The overarching goal is to transform Notepad++ from a tightly-coupled, Windows-only text editor into a **massive, cross-platform, multi-frontend "ultra-project."** 
Key transformations include:
*   **Total Win32 Decoupling:** Ripping out all `HWND`, `SendMessage`, and Windows API calls from the core business logic.
*   **The Go Port:** Systematically porting the entire backend and core architecture to Go, establishing it as the single source of truth for file state and configuration.
*   **Multiple Frontends (Submodules):** Using Go (or the abstracted C++ core) to drive multiple native UI bindings simultaneously, specifically `bobui` (Qt6), `btk` (Qt4), and `bobgui` (GTK).
*   **1:1 Geany Parity:** Building IDE-like features natively into the core, particularly a built-in terminal emulator and a compiler/build execution environment.

## 2. Core Architectural Decisions
*   **Event-Driven Architecture:** To break the Win32 `SendMessage` coupling, we are implementing a generic `EventBus` pattern. Both the transitional C++ core and the new Go backend use this to decouple Scintilla and the UI from the business logic.
*   **Abstracting Scintilla:** The legacy codebase communicates directly with Scintilla's internal window handle. We are routing all Scintilla interactions through an interface (`IScintillaBridge` in C++), which native UI wrappers will implement.
*   **Data Models First:** Data models like file buffering (`Buffer.cpp`) and configuration (`Parameters.cpp`) must be pure state, completely ignorant of the UI layer. We have successfully ported the `Buffer` and `BufferManager` concepts into Go (`go-port/pkg/core/buffer.go`).

## 3. Specific Feature Implementations
*   **Versioning Autosave:** Fulfilling the "Never lose a file" requirement, we implemented an immutable snapshotting system in Go (`pkg/autosave`). Instead of overwriting a temp file, it saves a ledger of all file states over time, deduplicating unchanged saves using SHA-256 hashes.
*   **Modern UI Demands:** The architecture is being prepped to support:
    *   **Vertical Tabs:** Modeled as adjustable-width columns rather than rotated 90-degree text.
    *   **Mixed Typography:** Structural support for rendering standard text with regular fonts and code with monospaced fonts seamlessly.
    *   **Plugin Integration:** Natively absorbing features from submodules like TextFX2 and Markdown viewing.

## 4. Operational & Agent Directives
*   **Documentation is Paramount:** Every architectural shift and version bump is meticulously documented across a suite of files (`VISION.md`, `ROADMAP.md`, `CHANGELOG.md`, `HANDOFF.md`, `SUBMODULES.md`, etc.).
*   **Strict Versioning:** Every commit and logical unit of work bumps the version (currently `v1.0.5`), tracked in the root `VERSION` file and `CHANGELOG.md`.
*   **Deep Commenting:** All touched and new code requires "deep comments" explicitly detailing the 'what', 'why', side effects, and design alternatives.
*   **Submodule Management:** External repositories are aggressively tracked as Git submodules with strict documentation of their current commit hashes and purposes.

## 5. Identified Anti-patterns in Legacy Code
*   **Styler Inheritance Breakage:** Hardcoding font styles directly into `stylers.model.xml` breaks Notepad++'s cascading style inheritance engine. This must be avoided in favor of the Style Configurator UI.
*   **Dialog Tying:** Logic is often embedded directly inside Dialog UI classes (`UserDefineDialog.cpp`, etc.), reading/writing directly to UI checkboxes instead of an underlying state model. This is the primary target for our decoupling efforts.

---