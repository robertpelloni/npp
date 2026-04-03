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

## Phase 3: Scintilla & Core Editor Engines (Pending)
- [ ] Embed native Scintilla HWND via `QWidget::createWindowContainer` or native overlay.
- [ ] Ensure Scintilla supports transparent background `SCI_STYLESETBACK` to preserve glass UI.
- [ ] Connect Lexilla syntax highlighting to Qt file extension events.
- [ ] Autocomplete popups and calltips glass styling.

## Phase 4: Plugin & Docking Systems (Pending)
- [ ] Port `QDockWidget` for Function List and Folder Workspace tools.
- [ ] Establish `QPluginLoader` system to replace `LoadLibrary` for NPP Plugins.
- [ ] Backward compatibility wrapper for legacy Win32 plugins (message hooking).
