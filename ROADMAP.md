# Notepad++ Modernization Roadmap

This roadmap outlines the long-term structural plans to achieve the goals defined in `VISION.md`. Because Notepad++ is a mature C++ Win32 application utilizing the Scintilla editing component, these changes require significant architectural updates.

## Phase 1: Contextual Intelligence (In Progress)
- [x] Disable autocomplete popup in standard non-code text files (L_TEXT, L_USER/Markdown).
- [ ] Investigate programmatic methods to set proportional fonts for prose and monospaced for code without breaking Scintilla's `stylers.xml` inheritance engine.

## Phase 2: "Never Lose a File" - Advanced Autosave & Versioning
- [ ] Design a silent, background local Git or SQLite-based versioning system.
- [ ] Hook into the Scintilla buffer modification events (`SCN_SAVEPOINTREACHED`, `SCN_MODIFIED`) to trigger silent snapshot saves.
- [ ] Build a UI viewer (perhaps a dockable panel) to view the history timeline of the current buffer.

## Phase 3: Core Feature Expansion & Plugin Absorption
- [ ] Audit the `textfx2` plugin. Port its string manipulation, encoding, and markup features directly into the core `PowerEditor/src/NppCommands.cpp`.
- [ ] Audit Geany features missing from Notepad++ (e.g., integrated VTE/Terminal, advanced symbol tree).
- [ ] Implement an integrated Terminal panel.

## Phase 4: UI Modernization (The "Glass" Era)
*Note: This is the most technically challenging phase due to Notepad++'s deep ties to pure Win32 API.*
- [ ] Research UI frameworks compatible with the existing architecture (e.g., custom drawing over Win32, or a migration to Qt/wxWidgets - though a framework migration would be a near-total rewrite).
- [ ] Implement a fully functional Vertical Tabs dockable pane, replicating all context menu actions of the standard horizontal tab bar.
- [ ] Implement a theming engine capable of skinning standard Win32 controls, borders, and scrollbars to achieve the "glass" look.
