package bindings

/*
// Deep comment: This CGO file bridges the Go backend to the C++ IScintillaBridge.
// Why: Go manages the state (Buffer, Config), but the actual text rendering happens in C++ (Qt/GTK + Scintilla).
//      We need a way for Go commands (like "TextFX.SortLines") to tell the C++ UI to replace text.
// Side effects: The C++ code MUST pass a struct of function pointers back to Go upon initialization.
//               Go stores these pointers globally, assuming exactly one active Scintilla editor per window.

#include <stdint.h>
#include <stdlib.h>

// This struct is passed FROM C++ INTO Go, so Go knows which C++ functions to execute.
typedef struct {
    void (*SetText)(const char* text);
    void (*InsertText)(int pos, const char* text);
    intptr_t (*SendScintillaMessage)(unsigned int message, uintptr_t wParam, intptr_t lParam);
} ScintillaFuncs;

// We store the pointer globally so CGO can jump back to the specific UI instance
static ScintillaFuncs g_scintillaFuncs;

// Set by Go when C++ passes the struct
static inline void RegisterScintillaFuncs(ScintillaFuncs funcs) {
    g_scintillaFuncs = funcs;
}

// Wrapper functions for Go to invoke the stored C++ pointers
static inline void CgoSetText(const char* text) {
    if (g_scintillaFuncs.SetText != NULL) {
        g_scintillaFuncs.SetText(text);
    }
}

static inline void CgoInsertText(int position, const char* text) {
    if (g_scintillaFuncs.InsertText != NULL) {
        g_scintillaFuncs.InsertText(position, text);
    }
}
*/
import "C"
import (
	"log"
	"unsafe"
)

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

func (b *ScintillaBridge) InsertText(position int, text string) {
	cStr := C.CString(text)
	defer C.free(unsafe.Pointer(cStr))
	C.CgoInsertText(C.int(position), cStr)
}

//export RegisterNativeScintilla
// RegisterNativeScintilla is called by the C++ UI to provide its specific Scintilla bridge hooks.
func RegisterNativeScintilla(funcs C.ScintillaFuncs) C.int {
	C.RegisterScintillaFuncs(funcs)
	log.Println("[CGO] Registered Native C++ Scintilla Bridge functions.")
	return 0
}
