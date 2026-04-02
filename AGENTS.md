# AI Agent Instructions

Welcome to the Notepad++ Modernization Project.

## Global Directives
1. **Scope Breakdown:** The user's vision is immense. You must read `VISION.md` and `ROADMAP.md`, select ONE isolated feature or bug from `TODO.md`, and complete it entirely before moving on.
2. **Win32 Constraints:** This is a legacy Win32 application. Do not propose or attempt to implement web-based UI paradigms (HTML/CSS) unless you are explicitly instructed to rip out the Win32 layer.
3. **Scintilla Awareness:** All text editing features must interact with the Scintilla component. Familiarize yourself with `SCI_*` messages.
4. **Performance:** Do not add heavy logic (string allocations, complex regex) to hot paths like `SCN_CHARADDED` or `AutoCompletion::update`.
5. **Styling Rules:** Do NOT hardcode font styles or colors across all nodes in `stylers.model.xml`. Notepad++ relies on a cascading style inheritance system (Global Override -> Default Style -> Lexer Node). Hardcoding breaks this system.
6. **Versioning:** Every time you submit a major feature, ensure the commit message references the feature and update `CHANGELOG.md` if necessary.

## Submodule Protocol
*Note: Depending on your sandbox environment, you may not have push access to external submodules. If requested to update submodules, verify your Git permissions first before attempting complex merge operations.*
