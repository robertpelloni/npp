# Scintilla CGO Boundary Planning

## Current State
The backend Go logic (`BufferManager`, `LSPManager`, `CommandManager`) dictates *when* text needs to change or *how* it should look. However, the physical text rendering component is still the native Scintilla C++ library, which lives inside the Native UI submodules (Qt/GTK).

## The Challenge
Go cannot directly invoke `SendEditor(SCI_INSERTTEXT)` because:
1. `HWND` does not exist cross-platform.
2. Go does not have memory access to the C++ Scintilla wrapper objects instance.

## The Solution: Two-Way CGO Bridge
We have established `pkg/bindings/scintilla_cgo.go` in Go, and `core/IScintillaBridge.h` in C++.

**Flow for Go mutating Scintilla:**
1. UI Frontend (`bobui/main.cpp`) instantiates a Scintilla widget.
2. UI Frontend implements `IScintillaBridge` mapping our clean methods to native Scintilla logic.
3. UI Frontend passes a pointer to this implementation into Go via an exported CGO setup function.
4. Go stores this C-pointer. When a Go command (e.g., `TextFX.SortLines`) completes, it calls `b.bridge->SetText(newText)`.

**Optimization Rules:**
- **Batch Updates:** Do *not* cross the CGO boundary character-by-character. Commands must format an entire buffer (or a large chunk) in Go, and make a single `SetText` or `ReplaceSelection` CGO call to Scintilla.
- **Async Execution:** `LSP` autocomplete lists must be formatted into JSON by Go, passed via the EventBus (`RegisterNativeEventListener`), and rendered by the C++ UI to prevent blocking the Go orchestrator loop.
