package bindings

/*
// Deep comment: This CGO file bridges the Go backend to the C++ IScintillaBridge.
// Why: Go manages the state (Buffer, Config), but the actual text rendering happens in C++ (Qt/GTK + Scintilla).
//      We need a way for Go commands (like "InsertText") to traverse the language boundary.
// Side effects: CGO introduces call overhead. Tight loops (like character-by-character parsing)
//               should happen purely in C++ or purely in Go, crossing the boundary only for bulk operations.

#include <stdint.h>
#include <stdlib.h>

// Forward declarations for C++ functions we will export from the native UI libs
extern intptr_t Native_SendScintillaMessage(unsigned int message, uintptr_t wParam, intptr_t lParam);
extern void Native_SetText(const char* text);
extern void Native_InsertText(int position, const char* text);

// Wrapper functions for CGO to call
static inline void CgoSetText(const char* text) {
    // Native_SetText(text); // Stubbed out until the C++ side is linked
}
*/
import "C"
import "unsafe"

// ScintillaBridge Go implementation wrapping the C++ calls
type ScintillaBridge struct{}

func NewScintillaBridge() *ScintillaBridge {
	return &ScintillaBridge{}
}

func (b *ScintillaBridge) SetText(text string) {
	cStr := C.CString(text)
	defer C.free(unsafe.Pointer(cStr))
	C.CgoSetText(cStr)
}
