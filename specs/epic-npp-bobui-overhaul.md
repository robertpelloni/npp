# Epic: Notepad++ UI Overhaul (BobUI) & TextFX Integration

## Overview
This epic covers the total architectural overhaul of Notepad++ from Win32 to the `bobui` (Qt6 equivalent) framework, including the native integration of the entire TextFX plugin codebase.

## Goals
- [ ] 100% feature parity with original NPP.
- [ ] Modernized, themeable UI using `bobui` Widgets and Styles.
- [ ] Native, high-performance implementations of all TextFX string/text routines.
- [ ] Cross-platform compatibility (Windows, Linux, macOS) via the BobUI layer.

## Key Sub-Tasks
1. **Infrastructure:** Integrate BobUI/Qt into the NPP build system.
2. **Main Window:** Port `Notepad_plus_Window` and `Notepad_plus` to `QMainWindow` and QObject-based logic.
3. **Editor Component:** Integrate Scintilla with BobUI (using `QScintilla` or custom bridge).
4. **Docking System:** Replace `DockingManager` with `bobui`'s native docking widgets.
5. **TextFX Port:** Port 380KB of `NPPTextFX.cpp` logic to `NppTextFXFeatures` C++ classes.
6. **Plugin Bridge:** Provide a Win32-emulator layer for existing DLL plugins or a new BobUI-based plugin API.

## Status
- [x] Initial Research & Submodule Setup
- [x] POC Design Created (`PowerEditor/src/NppBobUI_POC.cpp`)
- [ ] Architectural Design (Maestro Phase 1)
- [ ] Planning & Task Breakdown (Maestro Phase 2)
- [ ] Core Implementation
- [ ] TextFX Migration
- [ ] Testing & Validation
