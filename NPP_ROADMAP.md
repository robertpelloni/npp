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

## Phase 7: OS Integration & Advanced UX
- [ ] Implement System Tray icon with Glass context menu.
- [ ] Window Snap layout support for Windows 11.
- [ ] "Glass Splash" boot screen with dynamic loading progress.
- [ ] Multi-cursor focus tree virtualization for collaborative editing.
