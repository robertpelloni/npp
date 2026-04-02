# TODO List

Short-term tasks and actionable items derived from the Roadmap.

### UI / Aesthetics
- [ ] Research how to enable system-wide ClearType rendering overrides in Win32 C++ if not already natively handled by the OS.
- [ ] Prototype a custom drawn vertical tab bar list view.
- [ ] Identify which Win32 UI elements are currently hardcoded to system colors and need to be hooked into a custom theming engine.

### Versioning / Autosave
- [ ] Draft a data schema for storing file histories locally (e.g., `%APPDATA%/Notepad++/FileHistory`).
- [ ] Investigate performance implications of saving diffs vs. full file copies on every keystroke/pause.

### Plugins & Geany Parity
- [ ] List all individual functions provided by `textfx2`.
- [ ] Create a feature-comparison matrix between Notepad++ and Geany to identify exact gaps.

### Technical Debt / Refactoring
- [ ] The `AutoCompletion.cpp` file is heavily coupled with `NppParameters` and `ScintillaEditView`. Consider abstracting the logic.
