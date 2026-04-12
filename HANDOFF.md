# Notepad++ Project Handoff

This file tracks the latest actions and status for the next AI agent or human developer.

## Current Status
- Version 1.0.10
- Re-architected `pkg/autosave` to use a high-performance local SQLite database (`npp_versions.db`) instead of millions of flat files.
- `core/GoBridge.h` and `core/GoBridge.cpp` established as the Native C++ API for interacting with the Go backend via CGO.
- `ROADMAP.md` updated to mark Phases 1 & 2 complete. The primary focus is now firmly on Phase 3 (CGO integration and LSP).

## Recent Analysis & Decisions
- Utilizing `go-sqlite3` means the Go build process now strictly requires CGO. Since we also need CGO to export `ExecuteCommandFromUI` to the Qt/GTK Native frontends, this is fully acceptable and anticipated.
- The `GoBridge` C++ class serves as a singleton proxy. When the Qt UI needs to load a file, it calls `Core::GoBridge::GetInstance().ExecuteCommand("File.New")`.
- Next major hurdle is the LSP integration (replacing the legacy Scintilla autocomplete/intellisense plugin) to fulfill the final remaining massive Geany parity requirement.
