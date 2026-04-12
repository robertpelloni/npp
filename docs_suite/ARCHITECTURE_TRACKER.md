# Architecture & Library Tracker

This file tracks the overarching layout of the Notepad++ Ultra-Project architecture, showing where legacy C++ components map to their new Go counterparts.

| Legacy C++ Component | New Go Package (`go-port/pkg`) | Status | Notes |
| :--- | :--- | :--- | :--- |
| `WinMain` | `cmd/ultra/main.go` | **Complete** | Boots the Go models and CommandManager. |
| `NppBigSwitch.cpp` | `pkg/commands` | **Complete** | Replaced WM_COMMAND routing with Command Pattern. |
| `NppIO.cpp` | `pkg/io` | **Complete** | Abstracted Win32 `CreateFile` out of the codebase. |
| `Buffer.cpp` | `pkg/core` | **Complete** | Isolated document state from Scintilla. |
| `Parameters.cpp` | `pkg/config` | **Complete** | Centralized UI state toggles. |
| `TextFXEngine.h` | `pkg/textfx` | **Stubbed** | Native Go logic for sorting/alignment. |
| *(Plugin)* | `pkg/markdown` | **Stubbed** | Native Go markdown parsing engine. |
| *(New Feature)* | `pkg/autosave` | **Complete** | SHA-256 deduplicated versioning ledger. |
| *(New Feature)* | `pkg/workspace`| **Complete** | Vertical tabs layout management. |
| *(Geany Parity)* | `pkg/buildsys` | **Complete** | Async build execution. |
| *(Geany Parity)* | `pkg/terminal` | **Stubbed** | PTY interface. |
| `SendEditor(...)` | `pkg/bindings` | **Stubbed** | Scaffolding for CGO Scintilla calls. |
