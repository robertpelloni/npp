# Notepad++ Project Handoff

This file tracks the latest actions and status for the next AI agent or human developer.

## Current Status
- Version 1.0.17
- The project is officially in Phase 4. We are building the native UI wrappers.
- `bobui` (Qt6) has received `MockMainWindow.h`, which correctly implements the `UIWindow` loop and instantiates a `MockScintillaBridge.h` (in `core/`).
- The full CGO boundary is intact. C++ code can dispatch strings to the Go `CommandManager`, and Go commands can dispatch string mutations backwards over CGO into the specific instance of `MockScintillaBridge`.

## Recent Analysis & Decisions
- The architectural plumbing is complete. The system can successfully trigger a simulated UI "File.New" event or a simulated "TextFX.SortLines" event and pass the required text manipulations across the language barrier without throwing pointer exceptions or requiring the Win32 API.
- The immediate next hurdle is for a specialized Qt Developer to open the `bobui` submodule and replace `MockMainWindow.h` with an actual Qt Window containing a `QScintilla` widget, and link up the actual `QMenuBar` signals to `ExecuteCommand`.
