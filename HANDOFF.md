# Handoff: Notepad++ BobUI — Liquid Glass Edition v1.1.0

## Session Date
2026-04-03 | Agent: Antigravity (Google DeepMind Advanced Agentic Coding)

## Project Overview
A monumental architectural overhaul of Notepad++ (PowerEditor) from its legacy Win32
implementation to a modern, cross-platform UI using the `bobui` (Qt6 equivalent) framework,
now featuring a full **Liquid Glass** visual system inspired by Apple's visionOS/macOS design language.

---

## THIS SESSION: OS Polish, Symbol Categorization & Multi-Instance (v1.1.12)

### Major Achievements
1. **File Change Monitoring (Tail -f):** Implemented a native `QFileSystemWatcher` integration. The application now monitors all open files and prompts the user for a reload if an external process modifies the source on disk.
2. **Multi-Instance Support:** Added "Open in New Instance" to the File menu, allowing users to spawn independent `npp_liquid_glass` processes.
3. **Advanced Symbol Parsing:**
   * **Categorization:** Upgraded the `functionList.xml` and `GlassFunctionParser.h` to distinguish between **Classes** and **Functions**.
   * **Visuals:** The sidebar now displays type prefixes (e.g., `[Class] MyClass`) for better code navigation.
4. **Shortcut Mapper Automation:** The Shortcut Mapper now dynamically indexes every `QAction` registered in the `QMenuBar`, automatically populating the configuration table with current keybindings.
5. **Windows 11 Snap Layouts:** Added a `nativeEvent` filter to handle `WM_NCHITTEST` interceptions, paving the way for native snap menu support in the DWM-extended frame.
6. **Self-Update System:** Integrated a "Check for Updates" routine into the Help menu, providing a UI bridge for future GitHub-hosted release notifications.

---

### Files Modified & Created
| File | Changes |
|------|---------|
| `PowerEditor/src/NppLiquidGlass_Main.cpp` | Integrated File Watcher, New Instance, and Snap Layout stubs. |
| `PowerEditor/src/GlassShortcutMapper.h` | Upgraded to auto-populate from `QAction` lists. |
| `PowerEditor/src/GlassFunctionParser.h` | Injected symbol type categorization (Class vs Function). |
| `PowerEditor/src/functionList.xml` | Added class-level regex signatures. |
| `NPP_ROADMAP.md` | Marked Phase 12 as 90% Complete. |

---

## Build Instructions

```cmd
python build.py
```

**Executable output:** `build/Release/npp_liquid_glass_v2.exe`

---

## Next Steps for Implementor (Phase 15)

### Immediate (P0)
1. **Printing Support:** Resolve the `QtPrintSupport` static linkage to enable `actionPrint()`.
2. **Column Paste:** Implement a clipboard interceptor to handle rectangular data pasting into multiple Scintilla selections.

### Short-term (P1)
3. **Distribution Packaging:** Create an installer script (NSIS or InnoSetup) to bundle the executable with its necessary XML configuration files.
4. **Theme Store:** Prototype a `QNetworkAccessManager` bridge to download community-made `stylers.xml` themes.
