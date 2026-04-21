#pragma once

#include "GoBridge.h"
#include <string>

// Deep comment: UIWindow acts as the foundational abstraction for the UI submodules.
// Why: bobui (Qt6), btk (Qt4), and bobgui (GTK) all need to spawn a window and hook
//      into the Go EventBus. This interface defines the contract they must implement.
// Side effects: Enforces a strict boundary. The UI can *only* invoke commands via GoBridge
//               and *only* react to state via EventCallback execution.

namespace Core {

class UIWindow {
public:
    virtual ~UIWindow() = default;

    // Must be called to display the window and start the native UI message loop
    virtual int ShowAndRun() = 0;

    // Called by the Go EventBus callback when backend state mutates
    // e.g., "BufferManager.FileLoaded", "AppConfig.ThemeChanged"
    virtual void OnBackendStateChanged(const std::string& eventId, const std::string& jsonPayload) = 0;
};

} // namespace Core
