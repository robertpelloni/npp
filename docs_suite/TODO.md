# Notepad++ Short-Term TODO

This list tracks the granular, immediate tasks required to push the project forward.

## Done
- [x] Create comprehensive documentation suite (VISION, ROADMAP, CHANGELOG, etc.).
- [x] Add `robertpelloni/bobui` (Qt6), `btk` (Qt4), `bobgui` (GTK), `textfx` as submodules.
- [x] Map out the Win32 API calls inside `ScintillaComponent` (`PHASE_ANALYSIS.md`).
- [x] Abstract `EventBus` and `IScintillaBridge` in C++.
- [x] Port `Parameters`, `Buffer`, `IO`, `Commands` to pure Go.
- [x] Port `TextFX` sorting tools and `Markdown` parser foundations to Go.
- [x] Fulfill user UX: Vertical tabs layout configuration (`workspace/layout.go`).
- [x] Fulfill Geany Parity: `buildsys` and `terminal` scaffolding in Go.
- [x] **Versioning Autosave DB:** Migrated the flat-file Versioning Autosave to a robust SQLite local database (`github.com/mattn/go-sqlite3`).
- [x] **GoBridge C++ Scaffolding:** Created `core/GoBridge.h/cpp` as the stable C++ side of the CGO boundary.
- [x] **LSP Research & Foundation:** Geany's code intelligence must be matched or exceeded. Built a native Language Server Protocol (LSP) client in the Go backend (`pkg/lsp`) capable of executing/piping `gopls`, `clangd`, etc.
- [x] **LSP Routing:** Connected `BufferManager` events to the LSP client dynamically based on language type.
- [x] **Scintilla Roundtrip Architecture:** Implemented a Mock `IScintillaBridge` in C++ that prints buffer manipulations to the console to test the CGO boundary headless.
- [x] **Go CGO Scintilla Binding:** Exposed `RegisterNativeScintilla` so C++ UI components can pass a struct of mutating function pointers down to the Go core.
- [x] **UI Rendering Scaffolding:** Implemented a physical `MockMainWindow` stub inside `bobui` (Qt6). This GUI initializes the `GoBridge`, renders a text area (mock Scintilla), and binds a command to `ExecuteCommand`.
- [x] **Full Loop Verification:** Successfully verified that the Qt mock executes commands to Go, which manipulates the string, which travels back across CGO to the C++ Scintilla struct pointers.

## Next Up (Version 1.0.18 Focus)
- [ ] **Native Scintilla Abstraction:** The legacy `ScintillaEditView.cpp` is deeply coupled to Win32 `HWND`. We must create `core/NativeScintilla.h/.cpp` as a cross-platform implementation of `IScintillaBridge`.
- [ ] **UI Canvas Embedding:** The `bobui` (Qt6), `btk` (Qt4), and `bobgui` (GTK) applications must be able to embed this new platform-agnostic Scintilla wrapper natively into their respective canvas layers (`QWidget`, `GtkDrawingArea`).
