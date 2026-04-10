#pragma once

#include <string>
#include <cstdint>

namespace Core {

// Deep comment: IScintillaBridge abstracts the communication with the Scintilla editor control.
// Why: In the legacy codebase, Notepad++ sends messages directly to a Windows HWND (SendEditor).
//      To support Qt (bobui, btk) and GTK (bobgui), we cannot rely on HWND. The UI layer
//      must implement this interface and wrap the native Scintilla component.
// Side effects: Introduces virtual dispatch overhead for Scintilla calls, which is negligible
//               for GUI interactions but should be monitored for tight loop syntax highlighting.

class IScintillaBridge {
public:
    virtual ~IScintillaBridge() = default;

    // Core Scintilla Message passing, abstracting SendMessage
    virtual intptr_t SendScintillaMessage(unsigned int message, uintptr_t wParam = 0, intptr_t lParam = 0) = 0;

    // Convenience wrappers for common operations
    virtual void SetText(const std::string& text) = 0;
    virtual std::string GetText() const = 0;
    virtual void InsertText(int position, const std::string& text) = 0;
    virtual void SetLexer(int lexerId) = 0;
};

} // namespace Core
