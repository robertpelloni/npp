# Win32 Decoupling Analysis: ScintillaComponent & Commands

## Overview
The `PowerEditor/src/ScintillaComponent/` directory acts as the core controller between Notepad++'s business logic, the User Interface (dialogs), and the Scintilla text editing core. To successfully port to Go and decouple from Windows for Qt/GTK support, we must sever the Win32 ties here. Furthermore, `NppCommands.cpp` and `NppBigSwitch.cpp` represent the legacy event routing.

## Key Win32 Bindings Found
1.  **Dialog Management (`::SendMessage`, `::GetDlgItem`, `HWND`)**
    *   *Locations:* `UserDefineDialog.cpp`, `GoToLineDlg.cpp`, `FindReplaceDlg.cpp`
    *   *Usage:* Directly manipulating UI checkboxes, combo boxes, and text limits using Windows messages like `BM_GETCHECK`, `EM_LIMITTEXT`, and `CBN_SELCHANGE`.
    *   *Refactoring Strategy:* Dialog logic must be completely extracted. The core should only maintain a state structure (e.g., `UserDefineLanguageState`). The Qt/GTK frontend will bind its own comboboxes/checkboxes to this state structure.

2.  **Scintilla Interaction (`_pscratchTilla->execute()`)**
    *   *Locations:* Extensive use in `Buffer.cpp`, `ScintillaEditView.cpp`.
    *   *Usage:* `execute()` sends raw `SCI_*` messages. Scintilla maintains its own internal HWND.
    *   *Refactoring Strategy:* We have already created the `IScintillaBridge`. We need to convert `_pscratchTilla->execute(SCI_SETUNDOCOLLECTION, ...)` into `bridge->SendScintillaMessage(SCI_SETUNDOCOLLECTION, ...)`.

3.  **Command Routing (`NppBigSwitch.cpp`, `NppCommands.cpp`)**
    *   *Usage:* These files intercept raw `WM_COMMAND` and `WM_NOTIFY` messages from the Windows message loop and switch over hundreds of UI control IDs.
    *   *Refactoring Strategy:* We will port this to Go using the Command Pattern. Instead of a giant switch statement based on Windows IDs, the Go backend will register named Commands (e.g., "File.Save", "View.ToggleVerticalTabs") that the UI layer invokes by string identifier over CGO or the EventBus.

## Conclusion
The coupling is tightest in the Dialog classes (which are inherently UI and should be discarded/re-implemented in the UI submodules) and in the message routing loops.

Our immediate focus for the Go port will be defining a `CommandManager` that registers actions, allowing the UI to trigger them without knowing the underlying implementation or needing Win32 message IDs.
