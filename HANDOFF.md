# Notepad++ Project Handoff

This file tracks the latest actions and status for the next AI agent or human developer.

## Current Status
- Version 1.0.4
- The `go-port` directory has been initialized with a main entry point and the foundational `EventBus` logic, successfully tested.
- Submodules (`bobui`, `btk`, `bobgui`, `textfx`) are configured.
- Core interface abstractions (`EventBus`, `IScintillaBridge`, `VersioningAutosave`, `GeanyParity`) are designed in C++.

## Recent Analysis & Decisions
- The Go port is actively underway. We have decided to mirror the clean C++ decoupling logic (like EventBus) directly into idiomatic Go.
- Next steps involve systematically porting `Buffer.cpp`, `Parameters.cpp`, and other core parsing logic into Go packages (`pkg/core`), and exploring the `cgo` bindings required to link Go to the Qt/GTK submodules.
