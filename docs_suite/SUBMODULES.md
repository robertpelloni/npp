# Submodules and Project Architecture

This document tracks all external libraries and UI frameworks integrated into the project.

## Directory Structure
```
/
├── core/                (Future: Platform-agnostic C++ core logic)
├── go-port/             (Future: Go translation of the core)
├── bobui/               [Submodule] Qt6 UI components
├── btk/                 [Submodule] Qt4 UI components
├── bobgui/              [Submodule] GTK UI components
├── textfx/              [Submodule] TextFX plugins
├── docs_suite/          Comprehensive project documentation
└── PowerEditor/         Legacy Notepad++ Windows-specific source
```

## Integrated Submodules

| Submodule | Repository URL | Version (Commit Hash) / Date | Purpose / Role | Location in Project |
| :--- | :--- | :--- | :--- | :--- |
| **bobui** | `https://github.com/robertpelloni/bobui` | 78d7a58 / Fri Apr 3 13:54:55 2026 -0400 | Modern UI rendering using Qt6 | `/bobui` |
| **btk** | `https://github.com/robertpelloni/btk` | 5dc412d / Tue Apr 7 23:55:52 2026 -0400 | Legacy/fallback UI rendering using Qt4 (copperspice) | `/btk` |
| **bobgui** | `https://github.com/robertpelloni/bobgui` | 840e3a4 / Sat Apr 4 02:36:00 2026 -0400 | Native Linux UI rendering using GTK | `/bobgui` |
| **textfx** | `https://github.com/rainman74/NPPTextFX2` | 71ec1ea / Tue Feb 24 18:22:09 2026 +0100 | Advanced text manipulation plugin | `/textfx` |

## Architecture Notes
The overarching architecture demands that the core text editing logic (currently in `PowerEditor`) be abstracted away from Win32 APIs. Once abstracted, it will interface with the above submodules, allowing the user to compile Notepad++ with Qt6, Qt4, or GTK native frontends seamlessly.
