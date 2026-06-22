package main

import (
	"context"
	"fmt"
	"log"

	"github.com/notepad-plus-plus/ultra-project/pkg/bindings"
	"github.com/notepad-plus-plus/ultra-project/pkg/commands"
	"github.com/notepad-plus-plus/ultra-project/pkg/config"
	"github.com/notepad-plus-plus/ultra-project/pkg/core"
	"github.com/notepad-plus-plus/ultra-project/pkg/lsp"
	"github.com/notepad-plus-plus/ultra-project/pkg/plugins"
	"github.com/notepad-plus-plus/ultra-project/pkg/auth"
	"github.com/notepad-plus-plus/ultra-project/pkg/workspace"
	"github.com/notepad-plus-plus/ultra-project/pkg/textfx"
)

// Deep comment: This is the primary entry point for the Go-backed Notepad++ ultra-project.
// Why: Moving away from WinMain() in C++, we establish a clean Go runner.
//      Eventually, this main function will initialize the CGO UI bindings (Qt/GTK)
//      and boot up the core EventBus and Scintilla drivers.

// bridgeAdapter adapts the ScintillaBridge to the core.EditorUpdater interface.
type bridgeAdapter struct {
	*bindings.ScintillaBridge
}

func (a *bridgeAdapter) SetText(bufID core.BufferID, content []byte) {
	a.ScintillaBridge.SetText(string(content))
}

func (a *bridgeAdapter) InsertText(bufID core.BufferID, position int, text []byte) {
	a.ScintillaBridge.InsertText(position, string(text))
}

func main() {
	fmt.Println("Initializing Notepad++ Ultra-Project (Go Backend)...")

	// Initialize Core Systems
	eventBus := core.NewEventBus()
	bufManager := core.NewBufferManager(eventBus)

	// Initialize the Scintilla bridge and wire it to the buffer manager
	editorBridge := bridgeAdapter{bindings.NewScintillaBridge()}
	bufManager.SetEditorUpdater(&editorBridge)
	appConfig := config.DefaultConfig()
	// Load persisted config if it exists
	configPath := "config.xml"
	if err := appConfig.LoadXML(configPath); err != nil {
		log.Printf("[main] failed to load config (%s): %v (using defaults)", configPath, err)
	}
	defer func() {
		if err := appConfig.SaveXML(configPath); err != nil {
			log.Printf("[main] failed to save config: %v", err)
		}
	}()
	layout := workspace.NewLayout()

	_ = eventBus // will be passed to command manager

	// Initialize LSP Manager
	lspManager := lsp.NewManager(context.Background())

	// Initialize Command Router
	cmdManager := commands.NewManager()
	commands.RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)
	lsp.RegisterLSPCommands(cmdManager, lspManager, bufManager)
	textfx.RegisterTextFXCommands(cmdManager, bufManager)

	// Setup Authentication
	cmdManager.Use(auth.TokenMiddleware)
	cmdManager.Register(&commands.Command{
		ID:          "Auth.Login",
		Description: "Authenticate and retrieve a session token",
		Execute:     auth.Authenticate,
	})

	log.Println("Core data models and command router initialized.")

	// UI logic goes here

	// Example execution simulating a UI click
	log.Println("Simulating UI Action: File.New")
	if err := cmdManager.Execute("File.New", nil); err != nil {
		log.Fatalf("Command failed: %v", err)
	}

	activeBuf, _ := bufManager.GetActiveBuffer()
	log.Printf("Active buffer is now: %s", activeBuf.Filepath)

	// Load plugins from the "plugins" directory (if it exists)
	pluginMgr := plugins.NewManager()
	if err := pluginMgr.LoadFromDirectory("plugins"); err != nil {
		log.Printf("[main] plugin loading error: %v", err)
	}

	// TODO: Initialize native UI via CGO bindings

	log.Println("Backend initialized successfully. Awaiting Native UI hooks.")
}
