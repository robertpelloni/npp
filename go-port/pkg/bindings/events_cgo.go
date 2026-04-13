package bindings

/*
// Deep comment: This CGO file completes the round-trip boundary between Go and the C++ UI (Qt/GTK).
// Why: When Go mutates application state (e.g., BufferManager opens a file, AppConfig toggles a theme),
//      the Native UI needs to know about it so it can redraw. This allows C++ to pass function
//      pointers (callbacks) into the Go EventBus.
// Side effects: Crossing the CGO boundary with callbacks can be tricky regarding memory pinning.
//               We must pass simple C types (strings) rather than complex Go structs.

#include <stdint.h>
#include <stdlib.h>

// Typedef for the C++ function pointer we expect to receive
typedef void (*NativeEventCallback)(const char* eventId, const char* jsonPayload);

// Gateway function that CGO can call to invoke the C++ pointer
static inline void InvokeNativeCallback(NativeEventCallback cb, const char* id, const char* payload) {
    if (cb != NULL) {
        cb(id, payload);
    }
}
*/
import "C"
import (
	"encoding/json"
	"log"
	"unsafe"

	"github.com/notepad-plus-plus/ultra-project/pkg/core"
)

var globalEventBus *core.EventBus

// SetGlobalEventBus allows main.go to inject the singleton EventBus for CGO access.
func SetGlobalEventBus(eb *core.EventBus) {
	globalEventBus = eb
}

//export RegisterNativeEventListener
// RegisterNativeEventListener is called by the C++ GoBridge to listen to Go state changes.
func RegisterNativeEventListener(eventID *C.char, callback C.NativeEventCallback) C.int {
	if globalEventBus == nil {
		log.Println("[CGO Error] Global EventBus not initialized before C++ registration.")
		return 1 // Error
	}

	goEventID := C.GoString(eventID)

	globalEventBus.Subscribe(goEventID, func(payload interface{}) {
		// Convert whatever Go struct was passed into JSON
		jsonBytes, err := json.Marshal(payload)
		if err != nil {
			log.Printf("[CGO Error] Failed to marshal event payload for %s: %v\n", goEventID, err)
			return
		}

		cEventID := C.CString(goEventID)
		cPayload := C.CString(string(jsonBytes))
		defer C.free(unsafe.Pointer(cEventID))
		defer C.free(unsafe.Pointer(cPayload))

		// Jump back over the boundary to C++
		C.InvokeNativeCallback(callback, cEventID, cPayload)
	})

	log.Printf("[CGO] Registered Native C++ Event Listener for: %s\n", goEventID)
	return 0 // Success
}
