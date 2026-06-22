package main

import (
	"fmt"
	"github.com/notepad-plus-plus/ultra-project/pkg/core"
	"github.com/notepad-plus-plus/ultra-project/pkg/commands"
	"github.com/notepad-plus-plus/ultra-project/pkg/textfx"
)

func main() {
	eb := core.NewEventBus()
	bm := core.NewBufferManager(eb)
	cm := commands.NewManager()

	textfx.RegisterTextFXCommands(cm, bm)

	buf := bm.OpenBuffer("test.txt", "UTF-8")
	buf.Content = []byte("hello world")

	fmt.Printf("Before: %s\n", buf.Content)
	err := cm.Execute("TextFX.ConvertCaseUpper", nil)
	if err != nil {
		fmt.Printf("Error: %v\n", err)
	}
	fmt.Printf("After: %s\n", buf.Content)
}
