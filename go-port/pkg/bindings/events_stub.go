//go:build !cgo

package bindings

import "github.com/notepad-plus-plus/ultra-project/pkg/core"

// globalEventBus is a stub version used when CGO is disabled.
var globalEventBus *core.EventBus

// SetGlobalEventBus stores the event bus reference for later use.
func SetGlobalEventBus(eb *core.EventBus) {
	globalEventBus = eb
}
