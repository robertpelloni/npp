#pragma once

#include <string>
#include <vector>
#include <functional>

namespace Core {

// Deep comment: GeanyParity encapsulates features required for 1:1 Geany parity.
// Why: Geany acts as a lightweight IDE. Notepad++ needs internal Terminal and Build tools.
// Side effects: Running external processes requires careful threading to not block the main UI.

class BuildSystem {
public:
    using OutputCallback = std::function<void(const std::string&)>;

    // Executes a compiler or build script
    // Output is streamed back via the callback
    void ExecuteBuild(const std::string& command, const std::string& workingDir, OutputCallback onOutput);
};

class TerminalEmulator {
public:
    // Core logic for a PTY (pseudo-terminal)
    // The actual rendering will be handled by the Qt/GTK frontend.
    void SendInput(const std::string& input);
    void Resize(int cols, int rows);
};

} // namespace Core
