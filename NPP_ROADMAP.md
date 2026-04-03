# Notepad++ BobUI - Master Roadmap

## Phase 1: Prototype Foundation (Complete)
- [x] Integrate BobUI static libraries
- [x] Scaffold native C++ Qt6 QMainWindow to replace legacy wWinMain
- [x] Wire up MenuBar, Toolbar, and QTabWidget
- [x] Establish TextFXEngine.h (Native C++ string algorithms)
- [x] Scintilla/Lexilla mock placeholder (`QPlainTextEdit`)

## Phase 2: Liquid Glass UI System (Complete)
- [x] DWM Blur-behind integration (Acrylic / Mica)
- [x] `LiquidGlassPainter` multi-layer compositing
- [x] QSS theme tokens across all widgets
- [x] `GlassSettings.h` persistence via QSettings
- [x] `GlassPreferencesDialog` responsive frameless UI
- [x] Liquid bubble overlay animations
- [x] Macro recording state logic

## Phase 3: Scintilla & Core Editor Engines (Complete)
- [x] Embed native Scintilla HWND via native overlay (`BobScintilla` wrapping QWidget).
- [x] Ensure Scintilla supports transparent background `SCI_STYLESETBACK` to preserve glass UI (matched custom Liquid Glass dark-navy tint).
- [x] Wire text operations (Undo, Redo, Cut, Copy, Paste, Find/Replace, Go to Line) to Scintilla `SCI_*` messages.
- [x] Connect `SCN_MODIFIED` and `SCN_UPDATEUI` to Qt event signals (`nativeEvent`).

## Phase 4: Lexilla & Syntax Highlighting (Complete)
- [x] Statically load Lexilla engine.
- [x] Connect Lexilla syntax highlighting to Qt file extension events (C++, Python, JS, HTML).
- [x] Configure Scintilla code folding and margins.
- [x] Autocomplete popups and calltips glass styling.

## Phase 5: Plugin & Docking Systems (Complete)
- [x] Port `QDockWidget` for Function List and Folder Workspace tools.
- [x] Establish `QPluginLoader` system to replace `LoadLibrary` for NPP Plugins (Menu Scaffolded).
- [x] Backward compatibility wrapper for legacy Win32 plugins (message hooking).

## Phase 6: Core Feature Refinement & Polish (Complete)
- [x] Map "Find in Files" `QThread` pool async search backend to the Glass Find Dialog.
- [x] Integrated `GlassSearchWorker` (QThread) for directory recursive search.
- [x] Integrated `GlassFindInFilesDialog` frameless UI.
- [x] Built Search Results `QDockWidget` with double-click navigation to file/line.
- [x] Real-time parsing of legacy `themes/*.xml` color schemes into `SCI_STYLESET*` mappings via `GlassThemeManager`.
- [x] Implemented Scintilla Indicator Highlighting for Search Results.
- [ ] Connect native Qt QPrinter mapping for printing Scintilla documents.
- [ ] Complete `QPluginLoader` bridge injecting legacy `NppData` structs.

## Phase 10: OS Features & Terminal (Complete)
- [x] Engineered a native "Glass Terminal" utilizing `QProcess` and Scintilla for shell interaction.
- [x] Implemented XML configuration interop for NPP-compatible `shortcuts.xml` (Macro saving).
- [x] Built a high-fidelity "Shortcut Mapper" UI scaffold.
- [x] Refined Scintilla input trap for real-time shell TTY emulation.
- [ ] Connect native Qt QPrinter mapping for document printing.
- [ ] Implement Scintilla Marker logic for line-bookmarks (Margin 1).
- [ ] Implement Windows 11 Snap Layouts integration.

## Phase 11: Collaborative Singularity (Complete)
- [x] Engineered a "Glass Compare" tool utilizing side-by-side diffing and Scintilla markers.
- [x] Integrated vertical and horizontal scrolling sync into the native Compare workflow.
- [x] Built the `GlassUDLManager` for User Defined Language parsing.
- [x] Implemented "Recent Files" tracking with persistence and tooltips.
- [x] Integrated "Mark All" indicators (ID 9) for search visualization.

## Phase 12: Final Interop & Polish (Complete)
- [x] Engineered XML configuration interop for standard NPP `config.xml`.
- [x] Implemented "Select All Occurrences" via Scintilla multi-selection.
- [x] Integrated "Auto-insert Matching Braces/Quotes" logic.
- [x] Implemented professional "Line Operations" (Duplicate, Remove, Move Up/Down).
- [x] Unified document minimap toggles and status bar virtualization.
- [ ] Connect native Qt QPrinter mapping for document printing.
- [ ] Implement Windows 11 Snap Layouts integration via `WM_NCHITTEST`.

## Phase 13: The Collaborative Future (Complete)
- [x] Integrated real-time "Markdown Preview" dock utilizing Liquid Glass styling.
- [x] Engineered regex-based "Function List" parsing for C++, Python, and JS.
- [x] Implemented native `.nppw` Workspace files (JSON session serialization).
- [x] Added support for Multiple root folders in the Workspace Explorer.
- [x] Refined Scintilla edit traps for auto-pair matching and macro capture.
- [ ] Finalize WebSocket collaborative editing backend.
- [ ] Implement multi-cursor focus tree virtualization for shared sessions.
- [ ] Advanced UDL (User Defined Language) parser for custom XML syntax.

## Phase 8: Advanced OS Features (Complete)
- [x] Implemented Glass Command Palette (`Ctrl+Shift+P`) for rapid action searching.
- [x] Integrated `QSystemTrayIcon` with custom-painted Liquid Glass context menu.
- [x] Enhanced Status Bar with "Display Line" virtualization for Word Wrap modes.
- [x] Implemented "Restore" and "Minimize to Tray" functionality.

## Phase 9: Multi-Cursor & Collaboration (Complete)
- [x] Enabled multi-selection and rectangular selection (Column Mode) in Scintilla.
- [x] Implemented multiple root folder support for the Workspace Explorer.
- [x] Integrated vertical and horizontal synchronized scrolling for Dual View.
- [x] Built a "Glass Search Results" dock with highlighting indicators.
- [ ] Add support for Windows 11 Snap Layouts.
- [ ] Integrate a real-time collaborative editing backend (WebSocket/CRDT).
- [ ] Build a "Glass Terminal" dock widget.
