package integration

import (
	"sync"
	"testing"

	"github.com/notepad-plus-plus/ultra-project/pkg/commands"
	"github.com/notepad-plus-plus/ultra-project/pkg/config"
	"github.com/notepad-plus-plus/ultra-project/pkg/core"
	"github.com/notepad-plus-plus/ultra-project/pkg/workspace"
)

func TestSystemStabilityUnderLoad(t *testing.T) {
	eventBus := core.NewEventBus()
	bufManager := core.NewBufferManager(eventBus)
	appConfig := config.DefaultConfig()
	layout := workspace.NewLayout()
	cmdManager := commands.NewManager()
	commands.RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)

	const workers = 10
	const iterations = 500
	var wg sync.WaitGroup
	wg.Add(workers)

	for w := 0; w < workers; w++ {
		go func(id int) {
			defer wg.Done()
			for i := 0; i < iterations; i++ {
				// Mix of commands and events
				_ = cmdManager.Execute("File.New", nil)
				eventBus.Publish("StabilityTest", i)
			}
		}(w)
	}

	wg.Wait()
}
