# Notepad++ Ultra-Project Ideas

This document serves as a scratchpad for future improvements, refactoring ideas, and feature concepts generated during deep codebase analysis.

## Core Refactoring
*   **Event Bus:** Implement a robust, centralized event bus (Publish/Subscribe) in the new C++ core (and Go port) to entirely replace the tight coupling of Win32 `SendMessage` calls. This will allow the Qt/GTK frontends to listen for Scintilla events cleanly.
*   **SQLite for Versioning:** For the "Versioning Autosave" feature, rather than writing thousands of text files to a temp directory, consider using a local SQLite database to store file diffs/deltas efficiently over time.

## Feature Enhancements
*   **LSP Support Built-in:** Geany parity implies strong coding support. Instead of basic autocomplete, integrate the Language Server Protocol (LSP) directly into the core, allowing Notepad++ to act as a full IDE for any language.
*   **Git Integration Core:** Build git diffs, blame, and branch management directly into the vertical tab sidebar.

## Go Port Architecture
*   **Wails/Tauri Style:** For the Go port, evaluate if a Tauri-style architecture (Go backend, Web/Native UI frontend communicating via JSON-RPC) is cleaner than tight `cgo` bindings to C++ Qt libraries.
