# Notepad++ BobUI - Master Roadmap

## Phase 1: Prototype Foundation (Complete)
- [x] Integrate BobUI static libraries.
- [x] Scaffold native C++ Qt6 QMainWindow to replace legacy wWinMain.
- [x] Wire up MenuBar, Toolbar, and QTabWidget.
- [x] Establish TextFXEngine.h (Native C++ string algorithms).
- [x] Scintilla/Lexilla mock placeholder (QPlainTextEdit).

## Phase 2: Liquid Glass UI System (Complete)
- [x] DWM Blur-behind integration (Acrylic / Mica).
- [x] LiquidGlassPainter multi-layer compositing.
- [x] QSS theme tokens across all widgets.
- [x] GlassSettings.h persistence via QSettings.
- [x] GlassPreferencesDialog responsive frameless UI.
- [x] Liquid bubble overlay animations.

## Phase 3: Scintilla Core Engine (Complete)
- [x] Embed native Scintilla HWND via BobScintilla wrapper.
- [x] Ensure Scintilla supports transparent background SCI_STYLESETBACK.
- [x] Wire core text operations (Undo, Redo, Cut, Copy, Paste).
- [x] Connect SCN_MODIFIED and SCN_UPDATEUI to Qt signals.

## Phase 4: Lexilla & Syntax Highlighting (Complete)
- [x] Statically load Lexilla parsing engine.
- [x] Connect Lexilla highlighting to Qt file extension events.
- [x] Configure Scintilla code folding and margins.
- [x] Autocomplete popups and calltips glass styling.

## Phase 5: Plugin & Docking Systems (Complete)
- [x] Port QDockWidget for side-panels.
- [x] Establish GlassPluginManager for native DLL loading (NppData bridge).
- [x] Backward compatibility wrapper for legacy Win32 plugins.
- [x] Engineered "Plugin Toolbar Button" injection API.

## Phase 6: Search & Navigation (Complete)
- [x] Map "Find in Files" QThread pool async search backend.
- [x] Integrated GlassSearchWorker (QThread) for recursive search.
- [x] Built "Glass Search Results" dock with highlighting indicators.
- [x] Refactored Search Results to support Multiple simultaneous Result Tabs.

## Phase 7: Advanced UX (Complete)
- [x] Implemented Glass Command Palette (Ctrl+Shift+P).
- [x] Integrated QSystemTrayIcon with custom glass menu.
- [x] Integrated GlassSplashScreen with boot progress telemetry.
- [x] Engineered regex-based "Function List" parsing.

## Phase 8: Dual View & Layouts (Complete)
- [x] Dual Tab Widgets in QSplitter architecture.
- [x] "Move to Other View" and "Clone to Other View" logic.
- [x] Vertical and horizontal synchronized scrolling.
- [x] Distraction Free Mode toggle (Ctrl+Shift+D).

## Phase 9: Editing Powerhouse (Complete)
- [x] Enabled multi-selection and rectangular selection (Column Mode).
- [x] Integrated "Select All Occurrences" (Ctrl+Shift+L).
- [x] Integrated "Auto-insert Matching Braces/Quotes" logic.
- [x] Implemented professional "Line Operations" (Duplicate, Remove, Move).

## Phase 10: OS Integration (Complete)
- [x] Implemented native OS "Drag and Drop" for opening files.
- [x] Engineered a native "Glass Terminal" utilizing QProcess.
- [x] Enhanced Status Bar with "Display Line" virtualization.
- [x] Polished Scintilla visuals with "Caret Line Highlighting".

## Phase 11: Interoperability (Complete)
- [x] Engineered XML configuration interop for standard NPP config.xml.
- [x] Implemented XML macro persistence for NPP shortcuts.xml.
- [x] Implemented native .nppw Workspace files (JSON session serialization).
- [x] Built the GlassUDLManager for User Defined Language parsing.

## Phase 12: Geany & Plugin Parity (Complete)
- [x] Engineered native Build System with configurable commands (F8/F5).
- [x] Integrated Mime and Converter tools (Base64, Hex, URL).
- [x] Implemented XML-driven Function List parsing parity (Classes & Functions).
- [x] Added auto-closing tags for HTML/XML documents.
- [x] Integrated File Change Monitoring (Tail -f) with auto-reload.
- [x] Implemented "Open in New Instance" multi-process logic.
- [x] Built a "Check for Updates" system in the Help menu.
- [ ] Connect native Qt QPrinter mapping for document printing.
- [x] Implement Windows 11 Snap Layouts integration via WM_NCHITTEST.
- [x] Automated Shortcut Mapper population from indexed QActions.
