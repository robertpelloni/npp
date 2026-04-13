# Notepad++ Project Handoff

This file tracks the latest actions and status for the next AI agent or human developer.

## Current Status
- Version 1.0.14
- The legacy Win32/MSVC `CMakeLists.txt` has been bypassed with `CMakeLists.txt.ultra`, effectively breaking the last major tie to the legacy Windows compilation environment.
- The UI framework is actively stubbed out. The Qt6 repository (`bobui`) now contains `main.cpp`, which acts as the frontend executable linking to the `libultra.so` Go backend.

## Recent Analysis & Decisions
- At this point, the backend Go implementation and the CGO linkage are essentially complete for the current feature set (LSP, Autosave DB, UI Event Bus, Commands).
- The focus going forward is entirely dependent on building out the Native UI components inside `bobui`, `btk`, and `bobgui`, drawing Qt/GTK widgets that read the Go state and fire off the registered commands.
