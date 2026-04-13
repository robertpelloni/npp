# Notepad++ Ultra-Project Changelog

All notable changes to this project will be documented in this file. The versioning strictly follows Semantic Versioning.

## [1.0.16] - 2026-04-13
### Added
- Completed `SCINTILLA_PORTING.md` establishing the architectural boundaries for Scintilla CGO manipulation.
- Implemented `core/MockScintillaBridge.h`, a headless C++ mock to verify the backend Go commands (`pkg/bindings/scintilla_cgo.go`) can jump the CGO boundary and trigger C++ string manipulations.

## [1.0.15] - 2026-04-13
### Added
- Created `build.sh` as the master orchestrator to sequentially compile the Go backend into a `c-shared` library, configure the modern CMake UI targets, and build the respective Qt/GTK frontend executables.
- Stubbed out the native entry points (`main.cpp`) for the `btk` (Qt4) and `bobgui` (GTK) UI submodules, bringing them to parity with the `bobui` (Qt6) linkage to the Go backend.
- Added `SCINTILLA_PORTING.md` detailing the upcoming complex CGO boundary required to manipulate native C++ text editor state asynchronously from Go commands.

## [1.0.14] - 2026-04-13
### Added
- Replaced the legacy, Win32-locked MSVC build system with a modern, cross-platform `CMakeLists.txt.ultra`. This master script handles optionally building the native Qt6/Qt4/GTK frontends via flags like `-DBUILD_UI_QT6=ON`.
- Added the core UI event loop stub (`main.cpp`) into the `bobui` (Qt6) submodule. This native C++ code now dynamically links to the Go backend (`libultra.so`) at runtime and registers to receive `AppConfig.ThemeChanged` UI redraw events.

## [1.0.13] - 2026-04-13
### Added
- Successfully compiled the `go-port` backend into a C-shared dynamic library (`libultra.so`). This library encapsulates the CGO boundary and exposes `ExecuteCommandFromUI` and `RegisterNativeEventListener`.
- Updated the C++ `GoBridge` layer to utilize dynamic loading (`dlopen`/`dlsym`). This ensures the UI executables can launch safely and capture backend binding failures securely without crashing the OS loader.

## [1.0.12] - 2026-04-12
### Added
- Exposed the Go `EventBus` to C++ via `RegisterNativeEventListener` in `pkg/bindings/events_cgo.go`. The CGO bridge marshals internal Go states into JSON strings and triggers registered C++ function pointers, completing the reactive UI loop.
- Added `core/UIWindow.h`, an abstract C++ interface that forces any UI submodule to implement standard display and backend event listeners.

## [1.0.11] - 2026-04-12
### Added
- Developed native Language Server Protocol (`LSP`) foundations (`pkg/lsp`) directly into the Go backend.
- Implemented `LSPManager` to dynamically boot and route requests (e.g., `gopls`, `clangd`) over JSON-RPC based on the active `core.Buffer` language type.
- This ensures the Ultra-Project not only achieves 1:1 Geany parity but natively functions as a multi-language IDE without requiring external plugins like NppExec or legacy Ctags.

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
