# Notepad++ Ultra-Project Changelog

All notable changes to this project will be documented in this file. The versioning strictly follows Semantic Versioning.

## [1.0.10] - 2026-04-12
### Added
- Upgraded the flat-file Versioning Autosave logic in `pkg/autosave` to a robust local SQLite database using `github.com/mattn/go-sqlite3`. This resolves the issue of millions of snapshot files bloating the disk.
- Established `core/GoBridge.h` and `core/GoBridge.cpp` as the stable ABI boundary. The native Qt/GTK submodules will link against this to initialize the Go backend and dispatch commands to the Go `CommandManager`.
- Updated `ROADMAP.md` and `TODO.md` to reflect the transition from core foundational work into Phase 3 (CGO bindings and optimization).

## [1.0.9] - 2026-04-12
### Added
- Ported core `TextFX2` functionality into Go (`pkg/textfx`), including strings sorting tools directly accessible by the CommandManager.
- Abstracted `Markdown` viewer parsing into Go (`pkg/markdown`), establishing an independent translation layer for future UI binding.
- Implemented an `ARCHITECTURE_TRACKER.md` and `track_architecture.sh` to enforce oversight of Submodule versions against internal Go packages.

## [1.0.8] - 2026-04-12
### Added
- Ported core File I/O operations into `pkg/io/fileio.go`, providing a platform-agnostic way to read/write raw bytes independent of the Win32 API.
- Implemented Plugin Manager scaffolding (`pkg/plugins/manager.go`) establishing a generic Go interface for loading and managing extensions (like TextFX2) seamlessly across operating systems.

## [1.0.7] - 2026-04-12
### Added
- Created `pkg/commands` defining a `CommandManager` to replace legacy Win32 Message routing.
- Default commands (`File.New`, `View.ToggleVerticalTabs`) are now explicitly wired to the new Go state models (`BufferManager`, `AppConfig`, `Layout`).
- Established `pkg/bindings` directory for CGO interfaces bridging Go state/commands with C++ UI submodules (`bobui`, `btk`, `bobgui`) and Scintilla.

## [1.0.6] - 2026-04-12
### Added
- Ported `Parameters` configuration model to Go (`pkg/config/parameters.go`), adding native toggles for vertical tabs, typography mixing, and intelligent autocomplete behaviors.
- Established Geany parity interfaces in Go: `pkg/buildsys` for executing external builds and `pkg/terminal` for PTY terminal emulation.
- Created UI state data models (`pkg/workspace/layout.go`) enforcing Vertical Tabs as default with adjustable column widths.

## [1.0.5] - 2026-04-10
### Added
- Completed `PHASE_ANALYSIS.md` detailing Win32 coupling in `ScintillaComponent`.
- Implemented `autosave` package in Go with SHA-256 deduplication and continuous snapshotting.
- Ported core `Buffer` and `BufferManager` data models to `go-port/pkg/core`, fully decoupling state from Win32 file handles and Scintilla document pointers.

## [1.0.4] - 2026-04-10
### Added
- Initialized the `go-port` module.
- Created `cmd/ultra/main.go` entry point.
- Ported `EventBus` logic to idiomatic Go in `pkg/core/eventbus.go`.

## [1.0.3] - 2026-04-10
### Added
- Created `core/VersioningAutosave.h` establishing the interface for continuous file snapshotting.
- Created `core/GeanyParity.h` establishing abstractions for built-in Build Execution and PTY Terminal logic.

## [1.0.2] - 2026-04-10
### Added
- Created `core/` directory to house platform-agnostic business logic.
- Implemented `Core::EventBus` to replace Windows message passing coupling.
- Implemented `Core::IScintillaBridge` interface to abstract Scintilla HWND calls.

## [1.0.1] - 2026-04-10
### Added
- Added `robertpelloni/btk` as a git submodule for Qt4 UI rendering.
- Updated `SUBMODULES.md` with explicit commit hashes and dates for `bobui`, `btk`, `bobgui`, and `textfx`.

## [1.0.0] - 2026-04-10
### Added
- Comprehensive documentation suite (`VISION.md`, `CHANGELOG.md`, `ROADMAP.md`, `TODO.md`, `MEMORY.md`, `DEPLOY.md`, `IDEAS.md`, `AGENTS.md`).

### Changed
- Refactoring roadmap planned to decouple Win32 API.
