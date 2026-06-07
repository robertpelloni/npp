# Notepad++ Ultra-Project TODO

## Immediate Granular Tasks (Phase 5 Prep)

### Backend (Go-Port)
- [ ] **Optimize Undo/Redo:** Replace full-buffer snapshots in `BufferManager` with a delta-based or rope-based storage system to handle large files efficiently.
- [ ] **Implement LSP JSON-RPC:** Complete the `RequestCompletion` logic in `pkg/lsp/manager.go` to parse actual JSON-RPC responses from language servers.
- [ ] **Search Refinement:** Expose Regex capabilities in the `Search.Find` and `Search.Replace` commands, utilizing the existing `SearchService`.
- [ ] **Plugin System:** Implement the plugin loader stub in `main.go` to dynamically load `.so`/`.dll` plugins from a dedicated directory.
- [ ] **Configuration Persistence:** Ensure `AppConfig` changes are serialized to a local XML or SQLite database on exit.

### UI Integration (Native & BobUI)
- [ ] **Native Scintilla Wiring:** Bridge the `NativeScintilla` C++ implementation with the Go `BufferManager` via CGO.
- [ ] **Vertical Tab Interactions:** Replicate all context menu actions (Close, Close All, Pin) for the new Vertical Tab component in `bobui`.
- [ ] **Settings Panel Coverage:** Map every backend parameter in `AppConfig` to a UI toggle or input in the `SettingsPanel`.
- [ ] **Theme Engine:** Finalize the "Glass" skinning logic and ensure it applies to the Gio/Qt layers uniformly.

### Testing & Quality
- [ ] **LSP Mock Server:** Create a more robust mock LSP server for integration tests to verify multi-step JSON-RPC handshakes.
- [ ] **UI Automation:** Setup Playwright/Gio-equivalent UI automation to verify vertical tab transitions and modal dialogs.
- [ ] **Memory Audit:** Profile the `BufferManager` under high load with multiple 100MB+ files open.
