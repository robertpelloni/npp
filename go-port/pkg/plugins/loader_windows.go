//go:build windows
// +build windows

package plugins

import (
	"fmt"
	"path/filepath"
	"strings"
	"syscall"
	"unsafe"
)

// winPlugin wraps a Windows DLL loaded via syscall.
type winPlugin struct {
	name        string
	version     string
	dll         *syscall.LazyDLL
	initProc    *syscall.LazyProc
	shutdownProc *syscall.LazyProc
}

func (wp *winPlugin) Name() string    { return wp.name }
func (wp *winPlugin) Version() string { return wp.version }

func (wp *winPlugin) Init() error {
	if wp.initProc != nil {
		ret, _, err := wp.initProc.Call()
		if ret != 0 {
			return fmt.Errorf("native plugin %s init returned %d: %v", wp.name, ret, err)
		}
	}
	return nil
}

func (wp *winPlugin) Shutdown() error {
	if wp.shutdownProc != nil {
		wp.shutdownProc.Call()
	}
	return nil
}

// callStringFn calls a LazyProc that returns a *char and converts to Go string.
func callStringFn(proc *syscall.LazyProc) string {
	if proc == nil {
		return ""
	}
	ret, _, _ := proc.Call()
	if ret == 0 {
		return ""
	}
	return strings.TrimRight(func(ptr uintptr) string {
		if ptr == 0 {
			return ""
		}
		data := make([]byte, 0, 256)
		for i := 0; ; i++ {
			b := *(*byte)(unsafe.Pointer(ptr + uintptr(i)))
			if b == 0 {
				break
			}
			data = append(data, b)
		}
		return string(data)
	}(ret), "\r\n ")
}

func loadLibrary(libPath string) (Plugin, error) {
	absPath, err := filepath.Abs(libPath)
	if err != nil {
		return nil, fmt.Errorf("failed to resolve path %s: %w", libPath, err)
	}

	dll := syscall.NewLazyDLL(absPath)
	if dll == nil {
		return nil, fmt.Errorf("failed to load DLL: %s", absPath)
	}

	wp := &winPlugin{
		dll: dll,
	}

	// Try to get PluginName export
	nameProc := dll.NewProc("PluginName")
	wp.name = callStringFn(nameProc)
	if wp.name == "" {
		// Fall back to filename without extension
		wp.name = strings.TrimSuffix(filepath.Base(libPath), filepath.Ext(libPath))
	}

	// Try to get PluginVersion export
	verProc := dll.NewProc("PluginVersion")
	wp.version = callStringFn(verProc)
	if wp.version == "" {
		wp.version = "0.0.0"
	}

	// Try optional init/shutdown exports
	initProc := dll.NewProc("PluginInit")
	if initProc != nil && initProc.Find() == nil {
		wp.initProc = initProc
	}
	shutdownProc := dll.NewProc("PluginShutdown")
	if shutdownProc != nil && shutdownProc.Find() == nil {
		wp.shutdownProc = shutdownProc
	}

	return wp, nil
}
