package commands

import (
	"testing"

	"github.com/notepad-plus-plus/ultra-project/pkg/config"
	"github.com/notepad-plus-plus/ultra-project/pkg/core"
	"github.com/notepad-plus-plus/ultra-project/pkg/workspace"
)

func TestSearchFindLiteral(t *testing.T) {
	eventBus := core.NewEventBus()
	bufManager := core.NewBufferManager(eventBus)
	appConfig := config.DefaultConfig()
	layout := workspace.NewLayout()
	cmdManager := NewManager()
	RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)

	// Create buffer with content
	_ = cmdManager.Execute("File.New", nil)
	buf, _ := bufManager.GetActiveBuffer()
	buf.Content = []byte("hello world hello universe")

	// Search for literal "hello"
	var searchResults []core.SearchResult
	eventBus.Subscribe("Search.Results", func(payload interface{}) {
		searchResults = payload.([]core.SearchResult)
	})

	err := cmdManager.Execute("Search.Find", map[string]interface{}{"query": "hello"})
	if err != nil {
		t.Fatalf("Search.Find failed: %v", err)
	}

	if len(searchResults) != 2 {
		t.Errorf("Expected 2 results for 'hello', got %d", len(searchResults))
	}

	// Verify result positions
	if searchResults[0].Start != 0 || searchResults[0].End != 5 {
		t.Errorf("First result expected at [0,5), got [%d,%d)", searchResults[0].Start, searchResults[0].End)
	}
	if searchResults[1].Start != 12 || searchResults[1].End != 17 {
		t.Errorf("Second result expected at [12,17), got [%d,%d)", searchResults[1].Start, searchResults[1].End)
	}
}

func TestSearchFindRegex(t *testing.T) {
	eventBus := core.NewEventBus()
	bufManager := core.NewBufferManager(eventBus)
	appConfig := config.DefaultConfig()
	layout := workspace.NewLayout()
	cmdManager := NewManager()
	RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)

	_ = cmdManager.Execute("File.New", nil)
	buf, _ := bufManager.GetActiveBuffer()
	buf.Content = []byte("abc123 def456 ghi789")

	var searchResults []core.SearchResult
	eventBus.Subscribe("Search.Results", func(payload interface{}) {
		searchResults = payload.([]core.SearchResult)
	})

	// Search for digits using regex
	err := cmdManager.Execute("Search.Find", map[string]interface{}{
		"query": `\d+`,
		"regex": true,
	})
	if err != nil {
		t.Fatalf("Search.Find regex failed: %v", err)
	}

	if len(searchResults) != 3 {
		t.Errorf("Expected 3 digit groups, got %d", len(searchResults))
	}
}

func TestSearchFindRegexInvalid(t *testing.T) {
	bufManager := core.NewBufferManager(core.NewEventBus())
	appConfig := config.DefaultConfig()
	layout := workspace.NewLayout()
	cmdManager := NewManager()
	RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)

	_ = cmdManager.Execute("File.New", nil)

	// Invalid regex should return an error
	err := cmdManager.Execute("Search.Find", map[string]interface{}{
		"query": `[invalid`,
		"regex": true,
	})
	if err == nil {
		t.Error("Expected error for invalid regex, got nil")
	}
}

func TestSearchFindNoResults(t *testing.T) {
	eventBus := core.NewEventBus()
	bufManager := core.NewBufferManager(eventBus)
	appConfig := config.DefaultConfig()
	layout := workspace.NewLayout()
	cmdManager := NewManager()
	RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)

	_ = cmdManager.Execute("File.New", nil)
	buf, _ := bufManager.GetActiveBuffer()
	buf.Content = []byte("hello world")

	// Search state should be cleared when no results found
	err := cmdManager.Execute("Search.Find", map[string]interface{}{"query": "nonexistent"})
	if err != nil {
		t.Fatalf("Search.Find failed: %v", err)
	}

	// FindNext should error out since there are no results
	err = cmdManager.Execute("Search.FindNext", nil)
	if err == nil {
		t.Error("Expected error for FindNext with no results, got nil")
	}
}

func TestSearchFindNextPrev(t *testing.T) {
	eventBus := core.NewEventBus()
	bufManager := core.NewBufferManager(eventBus)
	appConfig := config.DefaultConfig()
	layout := workspace.NewLayout()
	cmdManager := NewManager()
	RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)

	_ = cmdManager.Execute("File.New", nil)
	buf, _ := bufManager.GetActiveBuffer()
	buf.Content = []byte("a b a b a")

	// Subscribe to CurrentResult events
	var currentResult core.SearchResult
	resultReceived := false
	eventBus.Subscribe("Search.CurrentResult", func(payload interface{}) {
		currentResult = payload.(core.SearchResult)
		resultReceived = true
	})

	// Initial search for "a" - should find 3 results, cursor at index 0
	err := cmdManager.Execute("Search.Find", map[string]interface{}{"query": "a"})
	if err != nil {
		t.Fatalf("Search.Find failed: %v", err)
	}

	if !resultReceived {
		t.Fatal("Expected CurrentResult event after Search.Find")
	}
	if currentResult.Start != 0 {
		t.Errorf("Expected first result at position 0, got %d", currentResult.Start)
	}
	resultReceived = false

	// FindNext should advance to index 1 (position 4)
	err = cmdManager.Execute("Search.FindNext", nil)
	if err != nil {
		t.Fatalf("Search.FindNext failed: %v", err)
	}
	if !resultReceived {
		t.Fatal("Expected CurrentResult event after FindNext")
	}
	if currentResult.Start != 4 {
		t.Errorf("Expected second result at position 4, got %d", currentResult.Start)
	}
	resultReceived = false

	// FindNext should advance to index 2 (position 8)
	err = cmdManager.Execute("Search.FindNext", nil)
	if err != nil {
		t.Fatalf("Search.FindNext failed: %v", err)
	}
	if currentResult.Start != 8 {
		t.Errorf("Expected third result at position 8, got %d", currentResult.Start)
	}
	resultReceived = false

	// FindNext should wrap around to index 0 (position 0)
	err = cmdManager.Execute("Search.FindNext", nil)
	if err != nil {
		t.Fatalf("Search.FindNext wrap failed: %v", err)
	}
	if currentResult.Start != 0 {
		t.Errorf("Expected wrapped result at position 0, got %d", currentResult.Start)
	}

	// FindPrev should go back to index 2 (position 8)
	resultReceived = false
	err = cmdManager.Execute("Search.FindPrev", nil)
	if err != nil {
		t.Fatalf("Search.FindPrev failed: %v", err)
	}
	if currentResult.Start != 8 {
		t.Errorf("Expected prev result at position 8, got %d", currentResult.Start)
	}
}

func TestSearchReplaceLiteral(t *testing.T) {
	eventBus := core.NewEventBus()
	bufManager := core.NewBufferManager(eventBus)
	appConfig := config.DefaultConfig()
	layout := workspace.NewLayout()
	cmdManager := NewManager()
	RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)

	_ = cmdManager.Execute("File.New", nil)
	buf, _ := bufManager.GetActiveBuffer()

	// Set initial content via ApplyEdit for proper delta tracking
	bufManager.ApplyEdit(buf.ID, 0, []byte{}, []byte("abc def abc def"))

	// Replace "abc" with "xyz"
	err := cmdManager.Execute("Search.Replace", map[string]interface{}{
		"query":       "abc",
		"replacement": "xyz",
	})
	if err != nil {
		t.Fatalf("Search.Replace failed: %v", err)
	}

	if string(buf.Content) != "xyz def xyz def" {
		t.Errorf("Expected 'xyz def xyz def', got '%s'", buf.Content)
	}
}

func TestSearchReplaceRegex(t *testing.T) {
	eventBus := core.NewEventBus()
	bufManager := core.NewBufferManager(eventBus)
	appConfig := config.DefaultConfig()
	layout := workspace.NewLayout()
	cmdManager := NewManager()
	RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)

	_ = cmdManager.Execute("File.New", nil)
	buf, _ := bufManager.GetActiveBuffer()

	// Set initial content
	bufManager.ApplyEdit(buf.ID, 0, []byte{}, []byte("hello 123 world 456"))

	// Replace all digits with "#"
	err := cmdManager.Execute("Search.Replace", map[string]interface{}{
		"query":       `\d+`,
		"replacement": "#",
		"regex":       true,
	})
	if err != nil {
		t.Fatalf("Search.Replace regex failed: %v", err)
	}

	expected := "hello # world #"
	if string(buf.Content) != expected {
		t.Errorf("Expected '%s', got '%s'", expected, buf.Content)
	}
}

func TestSearchReplaceRegexCaptureGroup(t *testing.T) {
	eventBus := core.NewEventBus()
	bufManager := core.NewBufferManager(eventBus)
	appConfig := config.DefaultConfig()
	layout := workspace.NewLayout()
	cmdManager := NewManager()
	RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)

	_ = cmdManager.Execute("File.New", nil)
	buf, _ := bufManager.GetActiveBuffer()

	// Set initial content
	bufManager.ApplyEdit(buf.ID, 0, []byte{}, []byte("hello world"))

	// Use capture groups in regex replace
	err := cmdManager.Execute("Search.Replace", map[string]interface{}{
		"query":       `(hello) (world)`,
		"replacement": "$2 $1",
		"regex":       true,
	})
	if err != nil {
		t.Fatalf("Search.Replace regex with capture groups failed: %v", err)
	}

	expected := "world hello"
	if string(buf.Content) != expected {
		t.Errorf("Expected '%s', got '%s'", expected, buf.Content)
	}
}

func TestSearchReplaceMissingParams(t *testing.T) {
	eventBus := core.NewEventBus()
	bufManager := core.NewBufferManager(eventBus)
	appConfig := config.DefaultConfig()
	layout := workspace.NewLayout()
	cmdManager := NewManager()
	RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)

	// Missing query should error
	err := cmdManager.Execute("Search.Replace", map[string]interface{}{
		"replacement": "xyz",
	})
	if err == nil {
		t.Error("Expected error for missing query parameter, got nil")
	}
}

func TestSearchFindNextBeforeFind(t *testing.T) {
	// Clear any leftover search state from previous tests in the same binary
	// (shared package-level variable causes test interference with same buffer IDs)
	searchStateMu.Lock()
	searchState = make(map[core.BufferID]*searchCursor)
	searchStateMu.Unlock()

	bufManager := core.NewBufferManager(core.NewEventBus())
	appConfig := config.DefaultConfig()
	layout := workspace.NewLayout()
	cmdManager := NewManager()
	RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)

	_ = cmdManager.Execute("File.New", nil)

	// FindNext without prior Find should error
	err := cmdManager.Execute("Search.FindNext", nil)
	if err == nil {
		t.Error("Expected error for FindNext without prior search, got nil")
	}
}
