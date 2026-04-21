#pragma once

#include "IScintillaBridge.h"
#include <iostream>

// Deep comment: A mock implementation of the Scintilla bridge for headless CGO testing.
// Why: Before Qt6/GTK can physically draw Scintilla widgets, we need to guarantee that
//      the Go backend can reliably call C++ functions to mutate the document buffer.
//      This class acts as a dummy receiver, printing Go's text-manipulation commands to stdout.
// Side effects: Contains no actual Scintilla HWND/internal logic. It exists purely for Phase 4 API validation.

namespace Core {

class MockScintillaBridge : public IScintillaBridge {
public:
    MockScintillaBridge() = default;
    ~MockScintillaBridge() override = default;

    intptr_t SendScintillaMessage(unsigned int message, uintptr_t wParam = 0, intptr_t lParam = 0) override {
        std::cout << "[MockScintilla] SendMessage ID: " << message
                  << " W: " << wParam << " L: " << lParam << std::endl;
        return 0; // Success
    }

    void SetText(const std::string& text) override {
        std::cout << "[MockScintilla] SetText called with: \n---\n" << text << "\n---" << std::endl;
    }

    std::string GetText() const override {
        return "Mocked Text Content";
    }

    void InsertText(int position, const std::string& text) override {
        std::cout << "[MockScintilla] InsertText at pos " << position
                  << ": " << text << std::endl;
    }

    void SetLexer(int lexerId) override {
        std::cout << "[MockScintilla] SetLexer ID: " << lexerId << std::endl;
    }
};

} // namespace Core
