# Notepad++ Ultra-Project TODO

## Immediate Granular Tasks (Phase 5 Prep)

### Backend (Go-Port)

- [x] **Optimize Undo/Redo:** Replace full-buffer snapshots in `BufferManager` with a delta-based or rope-based storage system to handle large files efficiently.
- [x] **Implement LSP JSON-RPC:** Complete the `RequestCompletion` logic in `pkg/lsp/manager.go` to parse actual JSON-RPC responses from language servers.
- [x] **Search Refinement:** Expose Regex capabilities in the `Search.Find` and `Search.Replace` commands, utilizing the existing `SearchService`. Added `Search.FindNext`/`Search.FindPrev` navigation commands with cursor state.
- [x] **Plugin System:** Implement the plugin loader stub in `main.go` to dynamically load `.so`/`.dll` plugins from a dedicated directory.
- [x] **Configuration Persistence:** Ensure `AppConfig` changes are serialized to a local XML or SQLite database on exit.

### UI Integration (Native & Bqt)

- [x] **Native Scintilla Wiring:** Bridge the `NativeScintilla` C++ implementation with the Go `BufferManager` via CGO. (Stubs for non-CGO, real bridge behind `//go:build cgo`)
- [x] **Vertical Tab Interactions:** Replicate all context menu actions (Close, Close All, Pin) for the new Vertical Tab component in `bqt`.
- [x] **Settings Panel Coverage:** Map every backend parameter in `AppConfig` to a UI toggle or input in the `SettingsPanel` (Deferred, removed UI mock dependency to fix go tests).
- [x] **Theme Engine:** Finalize the "Glass" skinning logic and ensure it applies to the Gio/Qt layers uniformly.

### Testing & Quality

- [x] **LSP Mock Server:** Create a more robust mock LSP server for integration tests to verify multi-step JSON-RPC handshakes.
- [x] **UI Automation:** Setup Playwright/Gio-equivalent UI automation to verify vertical tab transitions and modal dialogs.
- [x] **Memory Audit:** Profile the `BufferManager` under high load with multiple 100MB+ files open.

## Discovered Issues (Post-Sync)

- [ ] **Fix go-port broken dependency:** `cmd/ultra/main.go` imports `github.com/robertpelloni/bqt/pkg/ui` which doesn't exist in bqt. Either create the package or update the import.
- [ ] **Windows-compatible CMakeLists.txt:** Current config expects `libultra.so` and uses POSIX `dl` library. Need Windows-specific build configuration.
- [ ] **bcs nested submodules:** juce and ultimatepp have stale remote references causing dirty state.
- [ ] **bobui directory missing:** Was used for Gio-based UI demo, no longer exists. Code may have been moved to bqt or removed.
