package integration

import (
	"bufio"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"os"
	"strings"
	"testing"

	"github.com/notepad-plus-plus/ultra-project/pkg/commands"
	"github.com/notepad-plus-plus/ultra-project/pkg/config"
	"github.com/notepad-plus-plus/ultra-project/pkg/core"
	"github.com/notepad-plus-plus/ultra-project/pkg/lsp"
	"github.com/notepad-plus-plus/ultra-project/pkg/workspace"
)

func TestEndToEndFileWorkflow(t *testing.T) {
	// 1. Initialize System Components
	eventBus := core.NewEventBus()
	bufManager := core.NewBufferManager(eventBus)
	appConfig := config.DefaultConfig()
	layout := workspace.NewLayout()
	cmdManager := commands.NewManager()

	commands.RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)

	// 2. Subscribe to events for verification
	bufferOpened := false
	eventBus.Subscribe("BufferOpened", func(payload interface{}) {
		bufferOpened = true
	})

	// 3. Execute "File.New" command
	err := cmdManager.Execute("File.New", nil)
	if err != nil {
		t.Fatalf("File.New failed: %v", err)
	}

	// 4. Verify model states
	buf, err := bufManager.GetActiveBuffer()
	if err != nil {
		t.Fatalf("Failed to get active buffer: %v", err)
	}

	if buf.Filepath != "new_1.txt" {
		t.Errorf("Expected filepath new_1.txt, got %s", buf.Filepath)
	}

	if len(layout.Tabs) != 1 {
		t.Errorf("Expected 1 tab in layout, got %d", len(layout.Tabs))
	}

	if !bufferOpened {
		t.Error("Expected BufferOpened event to be published")
	}

	// 5. Execute "View.ToggleVerticalTabs" and verify config change
	initialPlacement := layout.Placement
	err = cmdManager.Execute("View.ToggleVerticalTabs", nil)
	if err != nil {
		t.Fatalf("View.ToggleVerticalTabs failed: %v", err)
	}

	if layout.Placement == initialPlacement {
		t.Errorf("Expected layout placement to toggle, but it stayed %s", layout.Placement)
	}

	// 6. Execute "File.Open" and verify
	// Create a dummy file for testing
	dummyPath := "test_open.txt"
	_ = os.WriteFile(dummyPath, []byte("test content"), 0644)
	defer os.Remove(dummyPath)

	err = cmdManager.Execute("File.Open", map[string]interface{}{"filepath": dummyPath})
	if err != nil {
		t.Fatalf("File.Open failed: %v", err)
	}

	activeBuf, _ := bufManager.GetActiveBuffer()
	if activeBuf.Filepath != dummyPath {
		t.Errorf("Expected active buffer to be %s, got %s", dummyPath, activeBuf.Filepath)
	}

	// 7. Execute "File.Save"
	activeBuf.IsDirty = true
	err = cmdManager.Execute("File.Save", nil)
	if err != nil {
		t.Fatalf("File.Save failed: %v", err)
	}
	if activeBuf.IsDirty {
		t.Error("Expected buffer to be clean after File.Save")
	}

	// 8. Verify Command Mapping
	if name, ok := commands.CommandIDToName[41001]; !ok || name != "File.New" {
		t.Errorf("Expected mapping for 41001 to be File.New, got %s", name)
	}

	// 9. Execute "Search.Find" and verify integration
	err = cmdManager.Execute("Search.Find", map[string]interface{}{"query": "test"})
	if err != nil {
		t.Fatalf("Search.Find failed: %v", err)
	}

	// 10. Execute "Search.Replace" and verify content update
	activeBuf.Content = []byte("hello world")
	err = cmdManager.Execute("Search.Replace", map[string]interface{}{"query": "world", "replacement": "golang"})
	if err != nil {
		t.Fatalf("Search.Replace failed: %v", err)
	}
	if string(activeBuf.Content) != "hello golang" {
		t.Errorf("Expected hello golang, got %s", activeBuf.Content)
	}

	// 11. Execute "File.CloseAll"
	err = cmdManager.Execute("File.CloseAll", nil)
	if err != nil {
		t.Fatalf("File.CloseAll failed: %v", err)
	}
	if len(layout.Tabs) != 0 {
		t.Errorf("Expected 0 tabs after CloseAll, got %d", len(layout.Tabs))
	}
}

func TestUndoRedoIntegration(t *testing.T) {
	eventBus := core.NewEventBus()
	bufManager := core.NewBufferManager(eventBus)
	appConfig := config.DefaultConfig()
	layout := workspace.NewLayout()
	cmdManager := commands.NewManager()
	commands.RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)

	// Create buffer
	_ = cmdManager.Execute("File.New", nil)
	activeBuf, _ := bufManager.GetActiveBuffer()

	// Use ApplyEdit for proper delta tracking (replaces old MarkDirty snapshot approach)
	// First, set initial content via ApplyEdit
	if err := bufManager.ApplyEdit(activeBuf.ID, 0, []byte{}, []byte("v2")); err != nil {
		t.Fatalf("ApplyEdit failed: %v", err)
	}

	if string(activeBuf.Content) != "v2" {
		t.Errorf("Expected v2, got %s", activeBuf.Content)
	}

	// Undo should revert to empty
	if err := cmdManager.Execute("Edit.Undo", nil); err != nil {
		t.Errorf("Edit.Undo execution failed: %v", err)
	}
	if string(activeBuf.Content) != "" {
		t.Errorf("Expected empty after undo, got %s", activeBuf.Content)
	}

	// Redo should restore v2
	if err := cmdManager.Execute("Edit.Redo", nil); err != nil {
		t.Errorf("Edit.Redo execution failed: %v", err)
	}
	if string(activeBuf.Content) != "v2" {
		t.Errorf("Expected v2 after redo, got %s", activeBuf.Content)
	}
}

func TestUIToBackendEventFlow(t *testing.T) {
	eventBus := core.NewEventBus()
	bufManager := core.NewBufferManager(eventBus)
	appConfig := config.DefaultConfig()
	layout := workspace.NewLayout()
	cmdManager := commands.NewManager()
	commands.RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)

	// Simulate UI tracking state change
	bufferChanged := false
	eventBus.Subscribe("BufferChanged", func(payload interface{}) {
		bufferChanged = true
	})

	// 1. Create a new buffer
	_ = cmdManager.Execute("File.New", nil)
	activeBuf, _ := bufManager.GetActiveBuffer()

	// 2. Simulate editing (Mocking UI activity)
	activeBuf.Content = []byte("user typed this")
	bufManager.MarkDirty(activeBuf.ID)

	// 3. Verify backend published the change event
	if !bufferChanged {
		t.Error("Expected UI to receive BufferChanged event after MarkDirty")
	}

	// 4. Simulate UI triggering Save
	err := cmdManager.Execute("File.Save", nil)
	if err != nil {
		t.Fatalf("File.Save failed: %v", err)
	}

	if activeBuf.IsDirty {
		t.Error("Expected buffer to be clean after UI-triggered save")
	}
}

func TestLSPIntegration(t *testing.T) {
	ctx := context.Background()
	eventBus := core.NewEventBus()
	bufManager := core.NewBufferManager(eventBus)
	appConfig := config.DefaultConfig()
	layout := workspace.NewLayout()
	cmdManager := commands.NewManager()
	lspManager := lsp.NewManager(ctx)

	commands.RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)
	lsp.RegisterLSPCommands(cmdManager, lspManager, bufManager)

	// 1. Create a buffer
	_ = cmdManager.Execute("File.New", nil)
	activeBuf, _ := bufManager.GetActiveBuffer()
	activeBuf.LanguageType = "mock_lang"

	// 2. Use a proper mock server via AddClient instead of 'cat'
	// Create a mock client with goroutine-based server that returns actual completions
	clientToServerReader, clientToServerWriter := io.Pipe()
	serverToClientReader, serverToClientWriter := io.Pipe()

	go func() {
		br := bufio.NewReader(clientToServerReader)
		for {
			// Read Content-Length header
			var contentLength int
			for {
				line, err := br.ReadString('\n')
				if err != nil {
					_ = serverToClientWriter.Close()
					return
				}
				line = strings.TrimRight(line, "\r\n")
				if line == "" {
					break
				}
				if strings.HasPrefix(line, "Content-Length:") {
					parts := strings.SplitN(line, ":", 2)
					if len(parts) == 2 {
						fmt.Sscanf(strings.TrimSpace(parts[1]), "%d", &contentLength)
					}
				}
			}

			body := make([]byte, contentLength)
			if _, err := io.ReadFull(br, body); err != nil {
				_ = serverToClientWriter.Close()
				return
			}

			// Parse request ID
			var req map[string]interface{}
			json.Unmarshal(body, &req)
			reqID := req["id"]

			// Build completion response with proper types
			items := []lsp.CompletionItem{
				{Label: "fmt.Println", Kind: 2, Detail: "func(a ...any) (n int, err error)"},
				{Label: "fmt.Sprintf", Kind: 2, Detail: "func(format string, a ...any) string"},
			}
			result := lsp.CompletionList{IsIncomplete: false, Items: items}
			resultJSON, _ := json.Marshal(result)

			// Use the proper Response struct so Result is json.RawMessage (not double-encoded)
			resp := lsp.Response{
				JSONRPC: "2.0",
				ID:      reqID,
				Result:  resultJSON,
			}
			respJSON, _ := json.Marshal(resp)

			header := fmt.Sprintf("Content-Length: %d\r\n\r\n", len(respJSON))
			serverToClientWriter.Write([]byte(header))
			serverToClientWriter.Write(respJSON)

			_ = serverToClientWriter.Close()
			return
		}
	}()

	mockClient := &lsp.Client{
		Language: "mock_lang",
		Command:  "mock",
	}
	mockClient.SetPipeIO(clientToServerWriter, bufio.NewReader(serverToClientReader))

	lspManager.AddClient("mock_lang", mockClient)

	// 3. Request completion
	err := cmdManager.Execute("LSP.Completion", map[string]interface{}{
		"line":      10,
		"character": 5,
	})
	if err != nil {
		t.Errorf("LSP.Completion failed: %v", err)
	}
}
