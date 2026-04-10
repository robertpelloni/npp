# Notepad++ Ultra-Project Roadmap

This document outlines the major, long-term structural plans to evolve Notepad++ into a multi-frontend, cross-platform, Go-backed ultra-project.

## Phase 1: Foundation and Decoupling
1.  **Documentation:** Establish the `VISION.md`, `CHANGELOG.md`, `TODO.md`, `ROADMAP.md` suite.
2.  **Win32 Isolation:** Extract the core business logic, Scintilla event handling, and data models away from Windows-specific UI calls (HWND, SendMessage, etc.).
3.  **Submodule Integration:** Add `bobui` (Qt6), `btk` (Qt4), `bobgui` (GTK), and `textfx` as git submodules. Establish centralized version tracking for these.

## Phase 2: Feature Injection & Parity
1.  **Geany Parity:** Implement a built-in terminal emulator and a compiler/build execution environment.
2.  **Core Plugins:** Integrate TextFX2 and a Markdown Viewer directly into the core engine.
3.  **UX Enhancements:**
    - Implement Versioning Autosave ("Never lose a file").
    - Implement Vertical Tabs with an adjustable-width column.
    - Implement font mixing (regular for text, monospaced for code).
    - Implement skinnable "modern glass" UI hooks.

## Phase 3: The Go Port
1.  **Go Backend Initialization:** Establish a Go project structure.
2.  **Logic Translation:** Methodically translate the decoupled C++ core logic into robust Go packages.
3.  **UI Binding:** Bind the Qt6, Qt4, and GTK submodules to the new Go backend (likely via cgo).

## Phase 4: Refinement and Domination
1.  **Submodule Assimilation:** Review all submodule repositories for missing features and incorporate them into the master ultra-project.
2.  **Web UI:** Develop a native web-based frontend driven by the Go backend.
