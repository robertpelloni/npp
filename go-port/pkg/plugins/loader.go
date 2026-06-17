package plugins

import (
	"fmt"
	"log"
	"os"
	"path/filepath"
	"strings"
)

// loadNativePlugin attempts to load a native shared library from the given path.
// Platform-specific implementations handle the actual loading.
func loadNativePlugin(libPath string) (Plugin, error) {
	return loadLibrary(libPath)
}

// LoadFromDirectory scans the given directory for platform-appropriate shared libraries
// and attempts to load each one as a plugin.
func (m *Manager) LoadFromDirectory(dir string) error {
	entries, err := os.ReadDir(dir)
	if err != nil {
		if os.IsNotExist(err) {
			log.Printf("[plugins] plugin directory %q does not exist, skipping", dir)
			return nil
		}
		return fmt.Errorf("failed to read plugin directory %s: %w", dir, err)
	}

	var loaded int
	for _, entry := range entries {
		if entry.IsDir() {
			continue
		}

		ext := strings.ToLower(filepath.Ext(entry.Name()))
		var validExt bool
		switch ext {
		case ".dll", ".so", ".dylib":
			validExt = true
		}

		if !validExt {
			continue
		}

		fullPath := filepath.Join(dir, entry.Name())
		plugin, err := loadNativePlugin(fullPath)
		if err != nil {
			log.Printf("[plugins] failed to load %s: %v", fullPath, err)
			continue
		}

		if err := m.LoadPlugin(plugin); err != nil {
			log.Printf("[plugins] failed to register %s: %v", plugin.Name(), err)
			continue
		}

		log.Printf("[plugins] loaded: %s v%s (%s)", plugin.Name(), plugin.Version(), fullPath)
		loaded++
	}

	log.Printf("[plugins] loaded %d plugin(s) from %s", loaded, dir)
	return nil
}
