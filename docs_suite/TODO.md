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

## Next Up (Version 1.0.10 Focus)
- [ ] **Versioning Autosave DB:** The current Go implementation writes flat snapshot files. This needs to be upgraded to a robust SQLite local database to efficiently query/manage millions of file versions without blowing up disk inodes.
- [ ] **GoBridge C++ Scaffolding:** Create `core/GoBridge.h/cpp` as the C++ side of the CGO interface. The legacy Notepad++ code (and the new submodules) must call this bridge to talk to `ExecuteCommandFromUI` in Go.
- [ ] **LSP Research:** Investigate the cleanest way to embed a Go-based Language Server Protocol (LSP) client to replace legacy autocomplete logic for Geany parity.
