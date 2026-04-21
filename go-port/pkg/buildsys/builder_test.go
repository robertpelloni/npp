package buildsys

import (
	"context"
	"strings"
	"sync"
	"testing"
	"time"
)

func TestBuildManager(t *testing.T) {
	bm := NewBuildManager()

	ctx, cancel := context.WithTimeout(context.Background(), 2*time.Second)
	defer cancel()

	var output []string
	var mu sync.Mutex

	// We use a simple echo command which is cross-platform enough for a basic unit test
	err := bm.ExecuteBuild(ctx, "echo", []string{"hello", "build", "system"}, ".", func(line string) {
		mu.Lock()
		defer mu.Unlock()
		output = append(output, line)
	})

	if err != nil {
		t.Fatalf("ExecuteBuild failed: %v", err)
	}

	mu.Lock()
	defer mu.Unlock()

	if len(output) == 0 {
		t.Fatal("Expected output, got none")
	}

	joined := strings.Join(output, " ")
	if !strings.Contains(joined, "hello build system") {
		t.Errorf("Expected output to contain 'hello build system', got: %v", joined)
	}
}
