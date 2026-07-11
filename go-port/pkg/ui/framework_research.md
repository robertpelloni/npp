# Phase 4 UI Framework Research

This document outlines the investigation into UI frameworks compatible with the existing Notepad++ architecture (C++ core, Scintilla component, and Go backend).

## Constraints
1.  **C++ Legacy Integration:** The framework must be able to embed or interact seamlessly with the existing `ScintillaEditView` (now being replaced by `NativeScintilla`) which is heavily reliant on Win32 concepts, even if abstracted.
2.  **Go Backend (`go-port`):** The framework must communicate effectively with the Go backend via CGO (`libultra.so`) and the JSON-RPC `EventBus`.
3.  **Cross-Platform (Target):** While Notepad++ is historically Windows-only, the presence of GTK (`bgtk`) and Qt (`bqt`) submodules indicates a strong desire for cross-platform support.
4.  **"Glass" Theme Support:** The framework needs to support advanced theming (custom drawing of controls, borders, scrollbars) to achieve the desired "Glass" look defined in Phase 4.

## Framework Candidates

### 1. Qt (Currently used in `bqt` submodule)
*   **Pros:**
    *   Excellent cross-platform support (Windows, macOS, Linux).
    *   Powerful styling engine (QSS - Qt Style Sheets) capable of achieving complex "Glass" themes.
    *   Robust signal/slot mechanism for event handling.
    *   Strong support for docking panels (QDockWidget), essential for IDE features like terminals and symbol trees.
    *   `QScintilla` exists as a native port/wrapper, making Scintilla integration relatively straightforward.
*   **Cons:**
    *   Large footprint.
    *   C++ based, requires careful CGO bindings (already partially established in `core/GoBridge.h/.cpp`).

### 2. GTK (Currently used in `bgtk` submodule)
*   **Pros:**
    *   Native look and feel on Linux.
    *   CSS-based theming engine.
    *   `ScintillaGTK` provides a solid integration path.
*   **Cons:**
    *   Historically less optimal performance/look on Windows compared to native or Qt.
    *   C-based API can be verbose, though CGO binding might be slightly easier than C++ virtual classes (as noted in MEMORY.md: "GTK frontend (`bobgui`) implements the CGO bridging layer via a C ABI (`BobguiCollabRenderer`) instead of C++ virtual classes").

### 3. Gio (UI in pure Go)
*   **Pros:**
    *   100% Go codebase, eliminating the need for complex CGO boundaries for the UI itself.
    *   GPU-accelerated, highly performant.
*   **Cons:**
    *   *Major Blocker:* Embedding a massive, complex C++ component like Scintilla into a pure Go Gio layout is incredibly difficult and likely to cause severe rendering and event loop conflicts. The project architecture relies on Scintilla for text editing.
    *   Younger ecosystem; complex docking layouts are harder to build out of the box compared to Qt.

### 4. Custom Win32 Draw (Legacy approach)
*   **Pros:**
    *   Zero additional dependencies.
    *   Direct integration with existing Notepad++ code.
*   **Cons:**
    *   *Major Blocker:* Defeats the cross-platform goals of the Go port.
    *   Achieving a modern "Glass" look with raw Win32 GDI/GDI+ is extremely tedious and prone to visual artifacts (flickering).

## Recommendation & Decision
The existing architecture correctly identifies **Qt (via the `bqt` submodule)** as the most viable path forward for the "Glass" era.

1.  **Integration:** Qt can wrap the native window handles required by Scintilla or use `QScintilla`.
2.  **Theming:** Qt Style Sheets (QSS) can directly ingest the properties exported by the newly implemented `go-port/pkg/theme` JSON engine (e.g., mapping `BackgroundColor` to `background-color` in QSS).
3.  **Layout:** `QDockWidget` natively solves the requirement for dockable Terminal and Timeline viewers.

## Next Steps for Phase 4
Focus efforts on the `bqt` submodule:
1.  Complete the CGO bindings for the `ThemeEngine` to export QSS to Qt.
2.  Implement the Vertical Tabs dockable pane using a custom styled `QListWidget` or `QTreeView`.
