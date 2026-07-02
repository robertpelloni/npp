# Project Memory & Observations

## Codebase Architecture
- **Language:** C++ (predominantly standard C++ and Win32 API).
- **Core Editor Component:** Scintilla (`ScintillaEditView`).
- **Configuration:** XML heavily used for syntax highlighting definitions (`langs.model.xml`), styling (`stylers.model.xml`), and settings.

## Design Preferences (Based on User Constraints)
- The user expects highly ambitious, modern features layered onto a classic application.
- The user is extremely enthusiastic but provides "wall of text" requirements that need to be carefully parsed and broken down into atomic, testable C++ commits.
- Performance is critical. When developing features (like autocomplete checking), do not introduce string allocations or heavy comparisons into hot loops (like keystroke handlers).

## Known Quirks
- The `make test` command is not natively configured in this repository for standard C++ unit tests. Verification often requires manual testing or analyzing the C++ logic carefully.
- "Markdown" does not exist as a core language; it is a User Defined Language (UDL).

## Scintilla Dual-Font Prose/Code Override
The programmatic method to set proportional fonts for prose and monospaced for code without breaking `stylers.xml` inheritance was implemented successfully as a proof-of-concept in `ScintillaEditView::setSpecialStyle` (around line 878 in `PowerEditor/src/ScintillaComponent/ScintillaEditView.cpp`).
By selectively overriding the `fontNameA` variable just before `execute(SCI_STYLESETFONT, ...)` based on the buffer's language type (`langType == L_TEXT || langType == L_USER`), we achieve the dynamic font injection while preserving all other inherited style attributes (colors, bold/italic, size) from the default styles.
The logic checks `_fontName.empty()` first, ensuring that only styles that explicitly define a font face receive the override, preventing `STYLE_DEFAULT` cascading issues.
