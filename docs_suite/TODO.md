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
- [x] **LSP Research & Foundation:** Geany's code intelligence must be matched or exceeded. We will build a native Language Server Protocol (LSP) client in the Go backend (`pkg/lsp`). It must spawn external language servers (like `clangd` or `gopls`) and communicate via JSON-RPC.
- [x] **LSP Routing:** Connect the `BufferManager` events to the LSP client so that opening a `.go` file dynamically boots `gopls`.

## Next Up (Version 1.0.16 Focus)
- [ ] **Scintilla Roundtrip Architecture:** Implement a Mock `IScintillaBridge` in C++ that prints buffer manipulations to the console.
- [ ] **Go CGO Scintilla Binding:** Just as C++ registers an EventListener to hear from Go, C++ must pass an `IScintillaBridge` instance pointer to Go. Go must save this pointer and use it to invoke text changes when a command like `TextFX.SortLines` completes.
