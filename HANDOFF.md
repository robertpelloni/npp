# Notepad++ Project Handoff

This file tracks the latest actions and status for the next AI agent or human developer.

## Current Status
- Version 1.0.9
- The Go backend now includes robust text manipulation logic (replacing `TextFX2`) inside `pkg/textfx`.
- A native Markdown parsing stub (`pkg/markdown`) is in place to decouple previewing logic from the Qt/GTK renderers.
- Added an automated `track_architecture.sh` to enforce the visualization of the Go architecture against submodule tracking versions (`ARCHITECTURE_TRACKER.md`).

## Recent Analysis & Decisions
- Phase 2 (Feature Injection & Parity) is largely functionally complete on the backend. We have established models for Vertical Tabs, Autosave, Geany Parity, and Core Plugins.
- Next steps should focus heavily on the C++ side of the CGO boundary. The Native UI libraries (`bobui`, `btk`, `bobgui`) now have a robust Go engine to back them up, but the C++ code needs to be adjusted to query the Go `CommandManager` and `EventBus` rather than listening to Win32 Messages.
