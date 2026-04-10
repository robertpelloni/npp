# Notepad++ Ultra-Project Changelog

All notable changes to this project will be documented in this file. The versioning strictly follows Semantic Versioning.

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
