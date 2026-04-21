package core

import (
	"sync"
	"testing"
)

func TestEventBus(t *testing.T) {
	eb := NewEventBus()
	called := false
	var wg sync.WaitGroup

	wg.Add(1)
	eb.Subscribe("test_event", func(payload interface{}) {
		called = true
		if payload.(string) != "test_data" {
			t.Errorf("Expected test_data, got %v", payload)
		}
		wg.Done()
	})

	eb.Publish("test_event", "test_data")
	wg.Wait()

	if !called {
		t.Error("Event callback was not called")
	}
}
