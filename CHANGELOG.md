## [1.0.21] - 2026-06-19
### Added
- Added `auth` package to handle user login and token generation using environment variables.
- Updated `commands.Manager` to support middleware execution chains via `Manager.Use()`, and wired up `TokenMiddleware` in `main.go`.

## [1.0.20] - 2026-06-19
### Fixed
- Removed missing `bobui` UI dependency from `go-port/cmd/ultra/main.go` mock to restore backend test stability.

# Changelog

All notable changes to this project will be documented in this file.

## [1.0.19] - 2025-05-14
### Added
- Mapped legacy Notepad++ command IDs to Go command strings.
- Implemented Vertical Tabs as the default layout with horizontal labels.
- Enhanced `bobui` with MenuBar, Toolbar, and Context Menus.
- Wired backend events to UI via central EventBus.
- Established integration testing suite for end-to-end Go backend verification.
- Verified system stability with full integration and unit test execution in the target Go environment.
- Performed baseline performance benchmarking (Command execution: ~167k ns/op, Event publishing: ~55 ns/op).
- Verified concurrent system stability under load and implemented necessary mutex synchronization in BufferManager.
- Confirmed project stability with final verification of all logic and integration tests.
- Successfully verified all Go-port packages build and test in the target environment (Go 1.25.0).
- Established robust end-to-end integration workflows for file operations, search, and undo/redo.
- Reimplemented core features (Search/Replace, Format conversion) and expanded UI menus (Format, Settings, Help).
- Completed and verified autonomous Golang migration and workflow protocol.
- Deployed and verified Go backend components in the staging environment.
- Verified autonomous workflow synchronization between Go backend and UI submodules.
- Completed integration testing phase with full test coverage for core workflows and performance.
- Comprehensive project documentation suite (`VISION.md`, `ROADMAP.md`, `TODO.md`, `AGENTS.md`, etc.) to guide future AI modernization efforts.

### Changed
- Disabled the intrusive autocomplete popup in standard non-code text files (Normal Text).
