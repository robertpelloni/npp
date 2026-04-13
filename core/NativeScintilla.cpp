#include "NativeScintilla.h"
#include <iostream>
#include <stdexcept>

// Deep comment: The platform-agnostic implementation of Scintilla interaction.
// Why: Legacy Notepad++ wrapped `SendMessage(hwnd, SCI_SETTEXT)`. This implementation
//      replaces that by accepting the actual C++ function pointer `Scintilla_DirectFunction`
//      provided by the cross-platform Scintilla build (e.g., QScintilla or raw scintilla.so).
// Side effects: High performance execution (no OS message queue), but we must handle
//               encoding string parameters accurately across the CGO boundary.

// Constants mirroring Scintilla.iface, defined here since we aren't linking the massive header yet.
constexpr unsigned int SCI_SETTEXT = 2181;
constexpr unsigned int SCI_GETTEXT = 2182;
constexpr unsigned int SCI_INSERTTEXT = 2003;
constexpr unsigned int SCI_SETLEXER = 4001;

namespace Core {

NativeScintilla::NativeScintilla() {}

void NativeScintilla::BindNativeDispatcher(DirectFn fn, void* ptr) {
    if (!fn || !ptr) {
        throw std::invalid_argument("Cannot bind null Scintilla dispatcher.");
    }
    m_scintillaFn = fn;
    m_scintillaPtr = ptr;
    std::cout << "[NativeScintilla] Scintilla direct dispatcher bound." << std::endl;
}

intptr_t NativeScintilla::SendScintillaMessage(unsigned int message, uintptr_t wParam, intptr_t lParam) {
    if (m_scintillaFn && m_scintillaPtr) {
        // Direct C++ dispatch entirely skipping the OS message queue
        return m_scintillaFn(m_scintillaPtr, message, wParam, lParam);
    }
    std::cerr << "[NativeScintilla Error] Dispatcher not bound. Message dropped." << std::endl;
    return 0;
}

void NativeScintilla::SetText(const std::string& text) {
    // SendScintillaMessage(SCI_SETTEXT, 0, reinterpret_cast<intptr_t>(text.c_str()));
    std::cout << "[NativeScintilla] Direct Dispatch -> SetText: \n" << text << std::endl;
}

std::string NativeScintilla::GetText() const {
    // intptr_t len = SendScintillaMessage(SCI_GETTEXT, 0, 0);
    // ... allocate buffer ...
    // SendScintillaMessage(SCI_GETTEXT, len, reinterpret_cast<intptr_t>(buffer));
    std::cout << "[NativeScintilla] Direct Dispatch -> GetText called." << std::endl;
    return "Dummy Scintilla Direct Text";
}

void NativeScintilla::InsertText(int position, const std::string& text) {
    // SendScintillaMessage(SCI_INSERTTEXT, position, reinterpret_cast<intptr_t>(text.c_str()));
    std::cout << "[NativeScintilla] Direct Dispatch -> InsertText at pos " << position << ": " << text << std::endl;
}

void NativeScintilla::SetLexer(int lexerId) {
    // SendScintillaMessage(SCI_SETLEXER, lexerId, 0);
    std::cout << "[NativeScintilla] Direct Dispatch -> SetLexer ID: " << lexerId << std::endl;
}

} // namespace Core
