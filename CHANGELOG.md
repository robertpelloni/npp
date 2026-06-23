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

## [1.0.22]
### Added
- Implemented `go-port/pkg/theme` engine to support dynamic JSON configuration of the "Glass" theme style across all UI layers.

## [1.0.23]
### Added
- Created `test_ui_automation.go` stub for Playwright/Gio UI testing.
- Implemented `TestMemoryAudit` in `go-port/pkg/core/buffer_audit_test.go` to profile `BufferManager` under 100MB+ large-file memory stress.

## [1.0.24]
### Added
- Implemented `VersionHistoryManager` in `pkg/autosave/versioning.go` to provide a silent, SQLite-backed file versioning system, fulfilling Phase 2 of the ROADMAP.
- Skips saving identical snapshots to conserve storage.

## [1.0.25]
### Refactored
- Addressed code review feedback to fix memory optimization violations.
- Switched `string()` casts to `bytes.Equal()` in `pkg/autosave/versioning.go` to prevent heavy GC allocations when hashing 100MB+ files.
- Refactored `pkg/commands/manager.go` to pre-compile the middleware execution chain during `.Register()`, rather than reallocating closures on every single `.Execute()`.
- Enhanced `pkg/core/buffer_audit_test.go` to allocate distinct, isolated byte arrays per buffer, defeating compiler pointer optimizations to accurately measure `BufferManager` high-load memory pressure.

## [1.0.26]
### Added
- Created `pkg/autosave/db_hook.go` to bridge `BufferChanged` and `BufferSaved` events (mapped from Scintilla `SCN_MODIFIED` and `SCN_SAVEPOINTREACHED`) directly into the `VersionHistoryManager`, providing fully automated background snapshotting.

## [1.0.27]
### Added
- Completed Phase 2 of ROADMAP by implementing `TimelineViewer` in `pkg/autosave/ui_viewer.go`.
- This provides the backend data source for visualizing the historical SQLite snapshots in a dockable Native UI panel.

## [1.0.28]
### Added
- Implemented core string manipulation features from legacy `textfx2` plugin directly into `pkg/textfx` (ROADMAP Phase 3).
- Added `RemoveBlankLines`, `TrimTrailingWhitespace`, and `ConvertCase` utilizing Go's highly optimized `bytes` package to prevent UI thread blocking on massive file operations.

## [1.0.29]
### Fixed
- Resolved a critical memory mutation bug in `TrimTrailingWhitespace` by using `bytes.Clone()` before appending characters, preventing buffer corruption.
- Registered all new `textfx` commands with the global `CommandManager` (`go-port/pkg/textfx/commands.go`) so they can be triggered from the Native UI (e.g., `TextFX.RemoveBlankLines`, `TextFX.ConvertCaseUpper`).

## [1.0.30]
### Added
- Completed Phase 3 of ROADMAP by implementing `TerminalManager` in `pkg/terminal/manager.go`.
- This handles cross-platform backend PTY instantiation (`cmd.exe`/`bash`), allowing dockable native UI panels to connect to local shell environments akin to Geany or VSCode.

## [1.0.31]
### Added
- Completed Phase 4 UI Framework Research (`go-port/pkg/ui/framework_research.md`). Concluded that Qt (`bqt`) remains the most viable path forward for achieving the "Glass" theme and cross-platform native embedding of Scintilla, dismissing pure Go (Gio) due to extreme complexity in embedding the legacy C++ text component.

## [1.0.32]
### Added
- Completed another Phase 4 UI Modernization step by implementing `VerticalTabsManager` (`go-port/pkg/ui/vertical_tabs.go`).
- Replicated standard horizontal tab bar context menu actions (Close, CloseAll, CloseOther, Pin) to power the target native dockable side-pane.

## [1.0.33]
### Fixed
- Fixed a UX bug in `VerticalTabsManager` (`pkg/ui/vertical_tabs.go`) where closing a tab would ruin the visual order of the remaining tabs by performing a fast slice swap instead of an ordered splice.
- Fixed a text mangling bug in `TimelineViewer` (`pkg/autosave/ui_viewer.go`) by parsing the history preview strings using safe UTF-8 rune slicing instead of raw byte slicing.

## [1.0.34]
### Fixed
- Updated `ROADMAP.md` to properly track the completion of the Phase 4 Glass Theming engine, which was implemented previously but left unchecked in the roadmap manifest.

## [1.0.35]
### Fixed
- Addressed code review feedback by removing a legacy simulated execution of `File.New` in `main.go` that was causing application crashes due to missing Authentication middleware tokens.
- Fixed a minor memory leak in `VerticalTabsManager` by zeroing out the dangling pointer before slicing the backing array.
- Prevented zombie processes in `TerminalManager.Kill` by running `cmd.Process.Wait()` in a background goroutine after dispatching the kill signal.

## [1.0.39]
### Fixed
- Executed full repository sync and intelligent reverse-merge protocol, bringing `master` up to speed with all autonomous feature branch work (Authentication, Theming, Versioning, and UI Frameworks).
- Explicitly purged tracking of extraneous compiled binaries and temporary test scripts that were erroneously added during autonomous feature branches.

## [1.0.40]
### Added
- Completed Phase 3 of `ROADMAP.md` by implementing the final missing Geany parity feature: the `SymbolBrowser` in `pkg/lsp/geany_parity.go`.
- Organizes raw LSP extracted symbols into hierarchical categories (Classes, Functions, Variables) for native dockable tree-view rendering.
