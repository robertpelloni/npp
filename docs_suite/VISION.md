# Notepad++ Ultra-Project Vision

## The Ultimate Goal
The ultimate goal of this project is to evolve Notepad++ from a Windows-only text editor into a massive, cross-platform, multi-frontend "ultra-project." We will port the core architecture to Go while maintaining the legacy C++ version, decouple all business logic from the Win32 API, and provide multiple native frontends using Qt6 (bobui), Qt4 (btk), and GTK (bobgui).

This project aims to achieve 100% 1:1 feature parity with Geany, seamlessly integrating an internal terminal, a native compiler/build execution environment, and adopting the best elements of all competitor text editors. Every single detail, menu item, and configuration option must be represented. If any implementation can be improved, it must be improved.

## Core Pillars
1.  **Architecture:**
    *   **Unified Core:** A strictly platform-agnostic core C++ library (and eventually a Go library) handling all Scintilla logic, file parsing, and data models.
    *   **Go Porting:** Methodically porting all robust logic to Go, aiming for a unified Go backend driving the multiple UI submodules.
    *   **Multiple Frontends:** Support for Qt6, Qt4, GTK, and eventually web-based UIs.

2.  **User Experience (UX) & UI Design:**
    *   **Modern Glass UI:** Fully skinnable interfaces featuring a "super modern glass" aesthetic.
    *   **Vertical Tabs:** Adjustable-width column for vertical tabs, discarding the sideways-rendered text of legacy vertical tabs.
    *   **Typography:** The ability to freely mix regular fonts for standard text files with monospaced fonts for code.
    *   **ClearType:** Enforced/enhanced ClearType rendering wherever possible.
    *   **Intelligent Popups:** Disabling autocomplete popups by default in non-code text files.

3.  **Features & Functionality:**
    *   **Versioning Autosave:** "Never lose any text file again." Every version of every file is autosaved continuously.
    *   **Geany Parity:** Built-in terminal and build tools.
    *   **Integrated Plugins:** Formerly external plugins (TextFX2, Markdown Viewer, etc.) are built directly into the core.

4.  **Extensive Documentation & Memory:**
    *   Every feature must be exhaustively documented in code, tooltips, manuals, and Help files.
    *   Rigorous tracking of submodules, library versions, and agent memory is mandatory.
