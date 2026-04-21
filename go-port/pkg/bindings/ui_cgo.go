package bindings

/*
// Deep comment: This CGO file establishes the entry points for the C++ UI (Qt/GTK) to call into Go.
// Why: When a user clicks a button in Qt, it needs to trigger a Go Command.
//      We export Go functions to C so the UI submodules can invoke them.

#include <stdint.h>
#include <stdlib.h>

// Typedefs for clarity
typedef void (*GoEventCallback)(const char* eventId, const char* payloadJson);
*/
import "C"
import (
	"fmt"
	"C"
)

// Exported function so C++ UI can trigger a Go command
//export ExecuteCommandFromUI
func ExecuteCommandFromUI(commandID *C.char) C.int {
	cmd := C.GoString(commandID)
	fmt.Printf("Received command from Native UI: %s\n", cmd)

	// In reality, this will route to the CommandManager we built earlier.
	// We'll return 0 for success, 1 for error.
	return 0
}
