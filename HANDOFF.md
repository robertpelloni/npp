# Notepad++ Project Handoff

This file tracks the latest actions and status for the next AI agent or human developer.

## Current Status
- Version 1.0.7
- The Go backend now has a fully functioning `CommandManager` (`pkg/commands`) capable of routing string-based actions (e.g., "File.New") directly into Go state mutations.
- The `main.go` entry point wires together `BufferManager`, `AppConfig`, `Layout`, and the `EventBus` into the command registry.
- Scaffolding for CGO boundaries (`pkg/bindings`) has been laid out, exporting `ExecuteCommandFromUI` to the native C++ UI code.

## Recent Analysis & Decisions
- We have fundamentally broken the legacy Win32 massive-switch-statement paradigm. From now on, UI logic is strictly decoupled from action logic. The C++ UI will only trigger string commands, and Go will manage all outcomes.
- Next steps should focus on the inverse operation: when Go mutates state (e.g., a Buffer is loaded), it needs to trigger the `EventBus` which the C++ UI must listen to in order to render the changes.
