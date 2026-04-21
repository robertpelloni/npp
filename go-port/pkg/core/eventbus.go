package core

import (
	"sync"
)

// Deep comment: Go translation of the EventBus mechanism.
// Why: Idiomatic Go prefers channels or synchronized maps over raw callbacks.
//      This struct mirrors the C++ EventBus behavior, providing a decoupled
//      way for Scintilla components, plugins, and UI layers to communicate.

type EventCallback func(payload interface{})

type EventBus struct {
	mu        sync.RWMutex
	listeners map[string][]EventCallback
}

func NewEventBus() *EventBus {
	return &EventBus{
		listeners: make(map[string][]EventCallback),
	}
}

func (eb *EventBus) Subscribe(eventID string, callback EventCallback) {
	eb.mu.Lock()
	defer eb.mu.Unlock()
	eb.listeners[eventID] = append(eb.listeners[eventID], callback)
}

func (eb *EventBus) Publish(eventID string, payload interface{}) {
	eb.mu.RLock()
	defer eb.mu.RUnlock()

	if callbacks, exists := eb.listeners[eventID]; exists {
		for _, cb := range callbacks {
			// In a high-perf scenario, consider firing these as goroutines,
			// though UI events often need strict ordering.
			cb(payload)
		}
	}
}
