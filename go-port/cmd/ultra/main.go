package main

import (
	"context"
	"fmt"
	"log"

	"github.com/notepad-plus-plus/ultra-project/pkg/auth"
	"github.com/notepad-plus-plus/ultra-project/pkg/bindings"
	"github.com/notepad-plus-plus/ultra-project/pkg/commands"
	"github.com/notepad-plus-plus/ultra-project/pkg/config"
	"github.com/notepad-plus-plus/ultra-project/pkg/core"
	"github.com/notepad-plus-plus/ultra-project/pkg/lsp"
	"github.com/notepad-plus-plus/ultra-project/pkg/plugins"
	"github.com/notepad-plus-plus/ultra-project/pkg/textfx"
	"github.com/notepad-plus-plus/ultra-project/pkg/workspace"
)

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

	eventBus := core.NewEventBus()
	bufManager := core.NewBufferManager(eventBus)

	editorBridge := bridgeAdapter{bindings.NewScintillaBridge()}
	bufManager.SetEditorUpdater(&editorBridge)

	appConfig := config.DefaultConfig()
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

	lspManager := lsp.NewManager(context.Background())
	cmdManager := commands.NewManager()

	commands.RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)
	lsp.RegisterLSPCommands(cmdManager, lspManager, bufManager)
	textfx.RegisterTextFXCommands(cmdManager, bufManager)

	cmdManager.Use(auth.TokenMiddleware)
	cmdManager.Register(&commands.Command{
		ID:          "Auth.Login",
		Description: "Authenticate and retrieve a session token",
		Execute:     auth.Authenticate,
	})

	log.Println("Core data models and command router initialized.")

	// UI logic goes here
	// Removing the mock "File.New" execution that was causing a log.Fatalf
	// due to the newly added Auth middleware requiring a token.

	pluginMgr := plugins.NewManager()
	if err := pluginMgr.LoadFromDirectory("plugins"); err != nil {
		log.Printf("[main] plugin loading error: %v", err)
	}

	log.Println("Backend initialized successfully. Awaiting Native UI hooks.")
}
