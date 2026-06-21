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

	// Create 100MB of dummy content
	largeContent := bytes.Repeat([]byte("A"), 100*1024*1024)

	var memStats runtime.MemStats
	runtime.GC()
	runtime.ReadMemStats(&memStats)
	startAlloc := memStats.Alloc

	// Open 3 large buffers
	for i := 0; i < 3; i++ {
		id := BufferID("audit_file_" + string(rune(i)))
		buf := bm.OpenBuffer(string(id), "UTF-8")
		buf.Content = largeContent
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
