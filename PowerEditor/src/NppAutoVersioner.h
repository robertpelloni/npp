#pragma once

#include "Scintilla.h"
#include "Notepad_plus_msgs.h"

// Deep comment: NppAutoVersioner hooks into the Scintilla modification event.
// Why: It serves as the listener for Phase 2 "Never Lose a File" features, triggering
//      asynchronous snapshot saves whenever a file is successfully saved or heavily modified.
// Side effects: We must avoid doing heavy operations directly on SCN_MODIFIED
//               to keep keystrokes fast.
class NppAutoVersioner {
public:
    static NppAutoVersioner& getInstance() {
        static NppAutoVersioner instance;
        return instance;
    }

    void handleModification(SCNotification* notification);

private:
    NppAutoVersioner() = default;
    ~NppAutoVersioner() = default;
};
