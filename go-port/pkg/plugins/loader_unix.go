//go:build unix || linux || darwin
// +build unix linux darwin

package plugins

import "fmt"

// stubPlugin represents a detected but non-loadable shared library on Unix.
// Full support requires CGO and dlfcn.h which may not be available in all build environments.
type stubPlugin struct {
	name    string
	version string
}

func (sp *stubPlugin) Name() string    { return sp.name }
func (sp *stubPlugin) Version() string { return sp.version }
func (sp *stubPlugin) Init() error     { return nil }
func (sp *stubPlugin) Shutdown() error { return nil }

func loadLibrary(libPath string) (Plugin, error) {
	// Unix shared library loading requires CGO (dlfcn.h).
	// As a stub, we return a plugin that can be registered but has no-op lifecycle.
	// For full support, build with CGO_ENABLED=1.
	return &stubPlugin{
		name:    libPath,
		version: "0.0.0",
	}, fmt.Errorf("Unix native plugin loading requires CGO: %s", libPath)
}
