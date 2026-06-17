//go:build !cgo

package bindings

import "log"

// ScintillaBridge is a stub implementation used when CGO is not available.
type ScintillaBridge struct{}

func NewScintillaBridge() *ScintillaBridge {
	log.Println("[bindings] ScintillaBridge stub (no CGO)")
	return &ScintillaBridge{}
}

func (b *ScintillaBridge) SetText(text string) {
	log.Printf("[bindings] ScintillaBridge.SetText stub (%d bytes)\n", len(text))
}

func (b *ScintillaBridge) InsertText(position int, text string) {
	log.Printf("[bindings] ScintillaBridge.InsertText stub (pos=%d, %d bytes)\n", position, len(text))
}
