package bindings

import (
	"testing"
	"github.com/notepad-plus-plus/ultra-project/pkg/core"
)

func TestEventBusCGOBindings(t *testing.T) {
	eb := core.NewEventBus()
	SetGlobalEventBus(eb)

	if globalEventBus == nil {
		t.Error("Expected globalEventBus to be initialized by SetGlobalEventBus")
	}
}
