#pragma once

#include "IScintillaBridge.h"
#include <string>

// Deep comment: NativeScintilla replaces the legacy `PowerEditor/src/ScintillaComponent/ScintillaEditView.cpp`.
// Why: The legacy code explicitly relied on HWND handles to dispatch `SCI_*` messages.
//      We need an abstract wrapper that Qt/GTK can embed inside a `QWidget` or `GtkDrawingArea`.
//      The UI frameworks will pass down a direct, platform-agnostic function pointer to their
//      local Scintilla instance's internal dispatch function `sptr_t (*fn)(void*, unsigned int, uptr_t, sptr_t)`.
// Side effects: We must avoid `windows.h` entirely. Character encoding must assume UTF-8 crossing the CGO boundary.

namespace Core {

class NativeScintilla : public IScintillaBridge {
public:
    // Typedef representing Scintilla's internal cross-platform direct dispatch function.
    // Instead of sending OS messages (like HWND SendMessage), we call the C++ method directly.
    typedef intptr_t (*DirectFn)(void* ptr, unsigned int iMessage, uintptr_t wParam, intptr_t lParam);

    NativeScintilla();
    ~NativeScintilla() override = default;

    // The UI Framework calls this to tell NativeScintilla how to reach the physical text editor
    void BindNativeDispatcher(DirectFn fn, void* ptr);

    // Overrides required by GoBridge (IScintillaBridge)
    intptr_t SendScintillaMessage(unsigned int message, uintptr_t wParam = 0, intptr_t lParam = 0) override;

    void SetText(const std::string& text) override;
    std::string GetText() const override;
    void InsertText(int position, const std::string& text) override;
    void SetLexer(int lexerId) override;

private:
    DirectFn m_scintillaFn = nullptr;
    void* m_scintillaPtr = nullptr;
};

} // namespace Core
