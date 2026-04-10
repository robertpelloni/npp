# Core Component

This directory represents the isolated, platform-agnostic core of the Notepad++ Ultra-Project.
Its purpose is to extract the core business logic, Scintilla event handling, and data models away from Windows-specific UI calls (HWND, SendMessage, etc.).

By maintaining a clean separation here, the UI frameworks (`bobui`, `btk`, `bobgui`) can link against this core library, and the Go porting effort can easily translate these isolated files one by one.

## Refactoring Guidelines
1. **No Win32 API calls (`<windows.h>`, `HWND`, `LRESULT`, `SendMessage`):** Replace Windows message passing with an internal Event Bus or direct observer patterns.
2. **Abstract Scintilla Communication:** Instead of sending `SCI_*` messages directly to a Windows handle, abstract Scintilla calls through an interface (`IScintillaBridge`) that the native UI bindings will implement.
3. **Data Models First:** `Buffer.cpp`, `Parameters.cpp`, and parsing logic should be completely decoupled from the UI.
