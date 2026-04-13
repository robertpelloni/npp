#pragma once

#include <string>
#include <functional>
#include <cstdint>

// Deep comment: GoBridge provides the C++ side of the CGO boundary.
// Why: The Qt (bobui, btk) and GTK (bobgui) native UI applications are written in C++.
//      They cannot compile Go directly. This header acts as the stable ABI they
//      include to initialize the Go backend and dispatch string-based commands
//      (e.g., "File.New") without knowing Go's internal mechanisms.
// Linkage: This will eventually be backed by a Go-generated C shared library or static archive.

namespace Core {

// Function pointer typedef expected by Go `RegisterNativeEventListener`
typedef void (*NativeEventCallback)(const char* eventId, const char* jsonPayload);

// Struct matching the CGO ScintillaFuncs definition in Go
struct ScintillaFuncs {
    void (*SetText)(const char* text);
    void (*InsertText)(int pos, const char* text);
    intptr_t (*SendScintillaMessage)(unsigned int message, uintptr_t wParam, intptr_t lParam);
};

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

    // Allow UI frameworks to listen to Go state changes (e.g., Theme changed, File loaded)
    bool RegisterEventListener(const std::string& eventID, NativeEventCallback callback);

    // Register the C++ Native Scintilla Bridge so Go commands can mutate the physical text buffer
    bool RegisterScintilla(const ScintillaFuncs& funcs);

    ~GoBridge();

private:
    GoBridge() = default;
    GoBridge(const GoBridge&) = delete;
    GoBridge& operator=(const GoBridge&) = delete;

    bool m_initialized = false;
    void* m_libHandle = nullptr;
    void* m_execCmdFunc = nullptr;
    void* m_regListenerFunc = nullptr;
    void* m_regScintillaFunc = nullptr;
};

} // namespace Core
