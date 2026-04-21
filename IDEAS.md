# Brainstorming & Ideas

This document contains ideas for improvements, refactoring, or alternative approaches discovered during development.

## UI Modernization Alternatives
Instead of trying to force pure Win32 API to render a "super modern glass" UI (which is notoriously difficult and error-prone on modern Windows versions):
- **Idea 1:** Explore wrapping the core Scintilla editing component in a modern framework like WinUI 3 / Windows App SDK, or Qt. This would be a massive undertaking but would immediately yield the "modern glass" (Mica/Acrylic) look and native dark mode for free.
- **Idea 2:** Implement a custom non-client area drawing technique (handling `WM_NCPAINT`, `WM_NCCALCSIZE`) to draw the glass frames, while using an existing lightweight skinning library (like Detours to hook drawing calls).

## "Never Lose a File" Implementation
- **Idea 1:** Instead of writing a custom diff engine, leverage `libgit2`. Treat the hidden `%APPDATA%` backup folder as a local git repository. Every auto-save creates a silent commit. This provides robust versioning, diffing, and compression for free.

## Styling Architecture
- **Idea 1:** As discovered during the autocomplete task, hardcoding font names into `stylers.model.xml` breaks Notepad++'s style inheritance (Global Override). To provide proportional text for prose and monospaced for code out-of-the-box, we should perhaps introduce a new concept in `NppParameters` / `ScintillaEditView` called "Font Classes" (e.g., `ProseFont`, `CodeFont`) and dynamically map Lexers to these classes at runtime, rather than static XML definitions.
