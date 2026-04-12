#pragma once

#include <string>

// Deep comment: GoBridge provides the C++ side of the CGO boundary.
// Why: The Qt (bobui, btk) and GTK (bobgui) native UI applications are written in C++.
//      They cannot compile Go directly. This header acts as the stable ABI they
//      include to initialize the Go backend and dispatch string-based commands
//      (e.g., "File.New") without knowing Go's internal mechanisms.
// Linkage: This will eventually be backed by a Go-generated C shared library or static archive.

namespace Core {

class GoBridge {
public:
    static GoBridge& GetInstance() {
        static GoBridge instance;
        return instance;
    }

    // Initialize the Go runtime, EventBus, Config, etc.
    bool Initialize();

    // Dispatch a command to the Go CommandManager.
    // e.g., ExecuteCommand("View.ToggleVerticalTabs")
    bool ExecuteCommand(const std::string& commandID, const std::string& jsonPayload = "{}");

private:
    GoBridge() = default;
    ~GoBridge() = default;
    GoBridge(const GoBridge&) = delete;
    GoBridge& operator=(const GoBridge&) = delete;

    bool m_initialized = false;
};

} // namespace Core
