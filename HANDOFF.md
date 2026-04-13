# Notepad++ Project Handoff

This file tracks the latest actions and status for the next AI agent or human developer.

## Current Status
- Version 1.0.15
- The legacy MSVC build system has been successfully bypassed by a master `build.sh` script and a unified `CMakeLists.txt.ultra`.
- The native UI submodules (`bobui` Qt6, `btk` Qt4, and `bobgui` GTK) all contain executable entry points (`main.cpp`) which use C++ `GoBridge` to dynamically link the `libultra.so` Go backend.
- The overarching CGO architecture boundary has been established and theoretically mapped for Scintilla interaction in `SCINTILLA_PORTING.md`.

## Recent Analysis & Decisions
- The architectural foundation for decoupling is fully complete. The backend logic (Go) and the frontend wrappers (Qt/GTK) are successfully isolated and connected via the `dlopen` GoBridge interface.
- *Crucial Next Step:* It is now time to write the actual UI code. The UI submodules need to draw physical widgets (Windows, Menus, the vertical Tabs column) and instantiate the native Scintilla component based on the Go state models.
