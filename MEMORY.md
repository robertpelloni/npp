# Project Memory & Observations

## 1. Core Architecture & Paradigm Shifts
* **Dual Backend Model:** The modernization effort maintains two parallel backends: a modernized C++ core for legacy integration and a new Golang port (`go-port/`) that acts as the heavy-lifting engine and collaboration hub.
* **C-Shared Library:** The `go-port/` compiles into a dynamic C-shared library (`libultra.so`).
* **Native Frontends:** Native UI submodules (`bqt` for Qt, `bgtk` for GTK) link to the Go backend at runtime via `core/GoBridge.h/.cpp` and communicate via a JSON-RPC `EventBus`.
* **CGO Boundary Best Practices:**
  * `import "C"` must be distinctly separated from other Go standard library and external module imports to avoid compilation failures.
  * When utilizing C standard library functions (like `C.free`), `#include <stdlib.h>` is required in the CGO preamble.
  * `C.int` (0 or 1) is strictly used over `C.bool` when passing booleans across the boundary to prevent CGO type linking errors.

## 2. Go Backend Replacements (`go-port/`)
The Go backend methodically replaces deeply coupled Win32 C++ legacy systems with modular, cross-platform Go packages:
* **`pkg/commands`:** Replaces the legacy giant `NppBigSwitch.cpp` with a dynamic `CommandManager`. It maps named command strings (e.g., "File.Save") instead of Windows Message IDs, and now supports an execution chain via `Manager.Use()` for middleware (e.g., Auth).
* **`pkg/auth`:** Handles authentication and token validation via `TokenMiddleware`. Commands like `Auth.Login` bypass middleware and validate credentials against environment variables (`ADMIN_USERNAME`, `ADMIN_PASSWORD`) to provision secure `crypto/rand` tokens.
* **`pkg/config`:** Replaces the Win32 XML `Parameters.cpp` parser, natively marshaling UI preferences into JSON.
* **`pkg/macros`:** Replaces `NppMacro.cpp`, executing JSON-serializable command streams.
* **`pkg/plugins`:** Uses Go's `plugin` package to dynamically load standard cross-platform extensions (.so, .dll), replacing the Win32 `PluginsManager`.
* **`pkg/textfx`:** Replaces the C++ NppTextFX plugin with optimized Go routines for string manipulation (sorting, hashing, stripping) executed entirely independent of the UI thread.
* **`pkg/search`:** Implements high-performance, concurrent 'Find in Files' using goroutines and `bufio.Scanner`, streaming results across CGO via `RegisterSearchResultCallback`.
* **`pkg/buildsys`:** Replicates Geany's compile functionality via a Build Action Matrix (`matrix.go`) exposed to the UI.
* **`pkg/terminal`:** Provides an embedded PTY using `os/exec` and `io.Pipes`, streaming I/O across CGO to dockable Qt/GTK panels.

## 3. Real-Time Collaborative Engine ("Glass")
* **WebSocket Hub:** A `net/http` WebSocket listener spawns dynamically on port `:8080/ws` when the CGO environment is initialized.
* **Operational Transformation (OT):** Used in `pkg/collab` to handle concurrent edits and ensure eventual consistency among connected peers.
* **C++ UI Intercepts:** `core/GlassCollabClient.h/.cpp` intercepts Scintilla `SCN_MODIFIED` and `SCN_UPDATEUI` events and pushes them to the Hub. Infinite echo loops are avoided by using `SCI_SETTARGETRANGE` / `SCI_REPLACETARGET` and filtering out `SC_MOD_CHANGEINDICATOR` events.
* **Continuous Autosave:** Incoming WebSocket collaborative edits are directly dispatched (`DispatchCollabEdit`) to the global SQLite Autosave engine (`pkg/autosave/db.go`) to guarantee deduplicated background snapshotting.
* **Multi-Cursor UI:** Remote coordinates pushed from the Go backend are visually rendered by mapping them directly to Scintilla Indicator elements.

## 4. LSP & Editor Intelligence
* **Language Server Protocol (`pkg/lsp`):** Dynamically spawns processes (`gopls`, `clangd`) with piped I/O.
* **Symbol Extraction:** Replicates Geany's Symbol Outline parity by broadcasting AST JSON strings to Native UIs via `OnSymbolsUpdatedCallback` (rendered into a `QTreeView` in Qt).

## 5. Development & UI Conventions
* **Scintilla Awareness:** All text editing features inherently interact with the Scintilla component (`SCI_*` messages). The active buffer's language type is represented internally (e.g., Markdown as a UDL: `L_USER`).
* **Styling Rules:** Hardcoding font styles directly into `stylers.model.xml` is considered an anti-pattern as it breaks the cascading style inheritance engine (Global Override -> Default Style -> Lexer Node).
* **CI/CD Constraints:** The standard `make test` command is unsupported. All Go tests must be run via `cd go-port && go test ./...`. `go-sqlite3` test files gracefully skip if `CGO_ENABLED=0`.

## 6. Strict Project Management Protocol
* **Autonomous Execution:** Agents are expected to operate on autopilot, execute sequential recommendations without pausing, and push to git autonomously.
* **Documentation Governance:** Extensive Markdown suites must be kept synchronized.
  * `TODO.md`: Granular short-term features and bug fixes.
  * `ROADMAP.md`: Long-term architectural milestones.
  * `CHANGELOG.md` / `VERSION`: Strict adherence to semantic version bumps across the project; commits must explicitly document the version bump.
  * `HANDOFF.md`: Critical context summaries prepared at the end of every active session to allow successor models to seamlessly resume context.
* **Pre-commit Rigor:** Always complete pre-commit instructions, run `go test ./...`, and properly merge upstream changes/submodules before declaring task completion.

## 7. Known Quirks & Actionable Resolutions
* **Missing Testing Frameworks (`make test` limitation):**
  * *Quirk:* The standard `make test` target is not natively configured in the repository for C++ components, forcing developers to rely exclusively on `go test ./...` in the `go-port/` directory.
  * *Resolution:* Introduce Google Test or Catch2 into the `CMakeLists.txt` and define a custom `test` target for the legacy C++ core, bridging the parity between Go and C++ unit tests.
* **Markdown Language Handling:**
  * *Quirk:* Markdown does not exist as a core Scintilla Lexer in the legacy system; it relies heavily on fragile User-Defined Language (UDL) logic (`L_USER`).
  * *Resolution:* Enhance `langs.model.xml` and the `NativeScintilla` implementation to formally recognize Markdown via the Scintilla Lexer framework (e.g., integrating the actual `lexilla` markdown plugin), bypassing UDL limitations entirely.
* **Keystroke Performance in TextFX/UI:**
  * *Quirk:* Intensive string manipulations (like case switching or regex replacements) during hot loops (like keystroke handlers) can cause micro-stutters if they rely on string allocations rather than byte slice manipulation.
  * *Resolution:* Ensure all incoming buffer modifications intercepted by `core/GoBridge` use pre-allocated buffers and zero-copy `bytes` conversions in Go, avoiding temporary `std::string` allocations in C++.
## Known Quirks
- The `make test` command is not natively configured in this repository for standard C++ unit tests. Verification often requires manual testing or analyzing the C++ logic carefully.
- "Markdown" does not exist as a core language; it is a User Defined Language (UDL).

## Scintilla Dual-Font Prose/Code Override
The programmatic method to set proportional fonts for prose and monospaced for code without breaking `stylers.xml` inheritance was implemented successfully as a proof-of-concept in `ScintillaEditView::setSpecialStyle` (around line 878 in `PowerEditor/src/ScintillaComponent/ScintillaEditView.cpp`).
By selectively overriding the `fontNameA` variable just before `execute(SCI_STYLESETFONT, ...)` based on the buffer's language type (`langType == L_TEXT || langType == L_USER`), we achieve the dynamic font injection while preserving all other inherited style attributes (colors, bold/italic, size) from the default styles.
The logic checks `_fontName.empty()` first, ensuring that only styles that explicitly define a font face receive the override, preventing `STYLE_DEFAULT` cascading issues.
