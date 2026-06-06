package integration

import (
	"testing"

	"github.com/notepad-plus-plus/ultra-project/pkg/commands"
	"github.com/notepad-plus-plus/ultra-project/pkg/config"
	"github.com/notepad-plus-plus/ultra-project/pkg/core"
	"github.com/notepad-plus-plus/ultra-project/pkg/workspace"
)

func BenchmarkCommandExecution(b *testing.B) {
	eventBus := core.NewEventBus()
	bufManager := core.NewBufferManager(eventBus)
	appConfig := config.DefaultConfig()
	layout := workspace.NewLayout()
	cmdManager := commands.NewManager()
	commands.RegisterDefaultCommands(cmdManager, bufManager, appConfig, layout)

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = cmdManager.Execute("File.New", nil)
	}
}

func BenchmarkEventPublishing(b *testing.B) {
	eb := core.NewEventBus()
	eb.Subscribe("TestEvent", func(payload interface{}) {})

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		eb.Publish("TestEvent", i)
	}
}

func BenchmarkSearchLiteral(b *testing.B) {
	s := core.NewSearchService()
	content := make([]byte, 1024*1024) // 1MB
	for i := range content {
		content[i] = 'a'
	}
	copy(content[512*1024:], []byte("target"))

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = s.FindAll(content, "target")
	}
}
