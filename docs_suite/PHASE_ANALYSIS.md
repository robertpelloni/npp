# Win32 Decoupling Analysis: ScintillaComponent

## Overview
The `PowerEditor/src/ScintillaComponent/` directory acts as the core controller between Notepad++'s business logic, the User Interface (dialogs), and the Scintilla text editing core. To successfully port to Go and decouple from Windows for Qt/GTK support, we must sever the Win32 ties here.

## Key Win32 Bindings Found
1.  **Dialog Management (`::SendMessage`, `::GetDlgItem`, `HWND`)**
    *   *Locations:* `UserDefineDialog.cpp`, `GoToLineDlg.cpp`, `FindReplaceDlg.cpp`
    *   *Usage:* Directly manipulating UI checkboxes, combo boxes, and text limits using Windows messages like `BM_GETCHECK`, `EM_LIMITTEXT`, and `CBN_SELCHANGE`.
    *   *Refactoring Strategy:* Dialog logic must be completely extracted. The core should only maintain a state structure (e.g., `UserDefineLanguageState`). The Qt/GTK frontend will bind its own comboboxes/checkboxes to this state structure.

2.  **Scintilla Interaction (`_pscratchTilla->execute()`)**
    *   *Locations:* Extensive use in `Buffer.cpp`, `ScintillaEditView.cpp`.
    *   *Usage:* `execute()` sends raw `SCI_*` messages. Scintilla maintains its own internal HWND.
    *   *Refactoring Strategy:* We have already created the `IScintillaBridge`. We need to convert `_pscratchTilla->execute(SCI_SETUNDOCOLLECTION, ...)` into `bridge->SendScintillaMessage(SCI_SETUNDOCOLLECTION, ...)`.

3.  **Cross-Component Notifications**
    *   *Locations:* `GoToLineDlg.cpp` calling `::SendMessage(_hParent, WM_NOTIFY, LINKTRIGGERED, ...)`
    *   *Refactoring Strategy:* Replace entirely with `Core::EventBus::Publish("LinkTriggered", payload)`.

## Conclusion
The coupling is tightest in the Dialog classes (which are inherently UI and should be discarded/re-implemented in the UI submodules) and in `Buffer.cpp` which intertwines file I/O with Scintilla document pointer management.

Our immediate focus for the Go port will be recreating `Buffer.cpp` (file loading, character decoding, state management) without *any* UI awareness.
