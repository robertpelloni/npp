# Agent Handoff Document

## Current Status
The project is currently undergoing a massive conceptual modernization based on the user's extreme requirements (glass UI, continuous versioning, massive feature integration).

Currently, only one specific bug/feature has been implemented in the codebase:
- **Completed:** Disabled the intrusive autocomplete popup in standard non-code text files. This was achieved by modifying `AutoCompletion::update` in `PowerEditor/src/ScintillaComponent/AutoCompletion.cpp` to return early if the active buffer's language is `L_TEXT`.

## Technical Findings & Constraints Discovered
1. **Scintilla Style Inheritance:** An attempt was made to hardcode `fontName="Consolas"` to all code lexers in `stylers.model.xml` to force monospace for code and proportional for text. This was reverted. **Do not hardcode styles across all nodes in the XML template.** Doing so breaks Notepad++'s Global Override cascading style engine.
2. **Language Identification:** Notepad++ uses `LangType` enums (like `L_TEXT`, `L_CPP`). Be aware that custom user languages (like Markdown) are not natively defined in the enum; they all fall under `L_USER`. Identifying a specific `L_USER` language requires querying the buffer's User Defined Language string name (which proved too costly to do on every keystroke in the autocomplete hook).

## Next Steps for Implementor Models
1. Review `VISION.md` and `ROADMAP.md` to understand the ultimate goals.
2. Pick an actionable item from `TODO.md` (Phase 2: Versioning or Phase 3: Plugin integration are the most viable starting points in the current C++ architecture).
3. If tackling UI modernization (Phase 4), read `IDEAS.md` first, as the Win32 constraints are severe.
