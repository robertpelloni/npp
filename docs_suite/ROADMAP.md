# Notepad++ Ultra-Project Roadmap

This document outlines the major, long-term structural plans to evolve Notepad++ into a multi-frontend, cross-platform, Go-backed ultra-project.

## Phase 1: Foundation and Decoupling (Complete)
- [x] **Documentation:** Establish the `VISION.md`, `CHANGELOG.md`, `TODO.md`, `ROADMAP.md` suite.
- [x] **Win32 Isolation:** Extract the core business logic, Scintilla event handling, and data models away from Windows-specific UI calls (HWND, SendMessage, etc.).
- [x] **Submodule Integration:** Add `bobui` (Qt6), `btk` (Qt4), `bobgui` (GTK), and `textfx` as git submodules. Establish centralized version tracking for these.

## Phase 2: Feature Injection & Parity (Backend Complete)
- [x] **Geany Parity:** Implement a built-in terminal emulator and a compiler/build execution environment (`pkg/buildsys`, `pkg/terminal`).
- [x] **Core Plugins:** Integrate TextFX2 (`pkg/textfx`) and a Markdown Viewer (`pkg/markdown`) directly into the core engine.
- [x] **UX Enhancements:**
    - Implement Versioning Autosave ("Never lose a file") (`pkg/autosave`).
    - Implement Vertical Tabs with an adjustable-width column (`pkg/workspace`).
    - Implement font mixing and skinnable UI hooks (`pkg/config`).

## Phase 3: The Go Port & CGO UI Binding (Active)
- [x] **Go Backend Initialization:** Establish a Go project structure (`go-port/`).
- [x] **Logic Translation:** Methodically translated decoupled C++ core logic into robust Go packages (Commands, IO, Buffer, Config).
- [x] **Data Optimization:** Migrate the flat-file Versioning Autosave to a robust SQLite database ledger to manage millions of diffs.
- [x] **UI Binding (CGO):** Implement `core/GoBridge.h` in C++ to act as the stable ABI boundary.
- [ ] **LSP Foundation:** Replace legacy autocomplete with a native Language Server Protocol (LSP) client inside Go to guarantee 1:1 Geany parity for code intelligence.

## Phase 4: Refinement and Domination (Future)
- [ ] **Submodule Assimilation:** Review all submodule repositories for missing features and incorporate them into the master ultra-project.
- [ ] **Web UI:** Develop a native web-based frontend driven by the Go backend.
