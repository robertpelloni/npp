package main

import (
	"fmt"
	"log"

	"github.com/notepad-plus-plus/ultra-project/pkg/commands"
	"github.com/notepad-plus-plus/ultra-project/pkg/config"
	"github.com/notepad-plus-plus/ultra-project/pkg/core"
	"github.com/notepad-plus-plus/ultra-project/pkg/workspace"
)

// Deep comment: This is the primary entry point for the Go-backed Notepad++ ultra-project.
// Why: Moving away from WinMain() in C++, we establish a clean Go runner.
//      Eventually, this main function will initialize the CGO UI bindings (Qt/GTK)
//      and boot up the core EventBus and Scintilla drivers.

func main() {
	fmt.Println("Initializing Notepad++ Ultra-Project (Go Backend)...")

	// Initialize Core Systems
	eventBus := core.NewEventBus()
	bufManager := core.NewBufferManager()
	appConfig := config.DefaultConfig()
	layout := workspace.NewLayout()

	_ = eventBus // will be passed to command manager

	// Initialize Command Router
	cmdManager := commands.NewManager()
	commands.RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)

	log.Println("Core data models and command router initialized.")

	// Example execution simulating a UI click
	log.Println("Simulating UI Action: File.New")
	if err := cmdManager.Execute("File.New", nil); err != nil {
		log.Fatalf("Command failed: %v", err)
	}

	activeBuf, _ := bufManager.GetActiveBuffer()
	log.Printf("Active buffer is now: %s", activeBuf.Filepath)

	// TODO: Load Plugins
	// TODO: Initialize native UI via CGO bindings

	log.Println("Backend initialized successfully. Awaiting Native UI hooks.")
}
