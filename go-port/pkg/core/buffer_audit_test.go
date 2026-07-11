package core

import (
	"bytes"
	"runtime"
	"testing"
)

// TestMemoryAudit profiles the BufferManager under high load with multiple 100MB files.
func TestMemoryAudit(t *testing.T) {
	if testing.Short() {
		t.Skip("Skipping memory audit in short mode.")
	}

	eb := NewEventBus()
	bm := NewBufferManager(eb)

	var memStats runtime.MemStats
	runtime.GC()
	runtime.ReadMemStats(&memStats)
	startAlloc := memStats.Alloc

	// Open 3 large buffers and assign unique, isolated byte slices to each
	// to properly test memory allocation without relying on single-pointer optimization.
	for i := 0; i < 3; i++ {
		id := BufferID("audit_file_" + string(rune(i)))
		buf := bm.OpenBuffer(string(id), "UTF-8")

		// Create a distinct 100MB allocation for each buffer
		buf.Content = bytes.Repeat([]byte("A"), 100*1024*1024)
	}

	runtime.ReadMemStats(&memStats)
	endAlloc := memStats.Alloc

	// Allow roughly 300MB + some overhead
	allocatedMB := (endAlloc - startAlloc) / 1024 / 1024
	if allocatedMB > 400 {
		t.Errorf("Memory audit failed: expected < 400MB, got %dMB", allocatedMB)
	}

	// Clean up
	for i := 0; i < 3; i++ {
		bm.CloseBuffer(BufferID("audit_file_" + string(rune(i))))
	}
}
