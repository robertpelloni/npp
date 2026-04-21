package io

import (
	"fmt"
	"io"
	"os"
)

// Deep comment: This package replaces the Win32 dependent NppIO.cpp.
// Why: Legacy Notepad++ uses Windows API calls (CreateFile, ReadFile) for operations.
//      By moving file I/O strictly into Go, we gain cross-platform compatibility implicitly.
//      This layer is responsible for reading raw bytes from disk; encoding detection
//      and decoding will be handled by a higher-level parsing package before reaching the Buffer.

type FileManager struct{}

func NewFileManager() *FileManager {
	return &FileManager{}
}

// ReadFile reads the entire contents of a file into a byte slice.
func (fm *FileManager) ReadFile(filepath string) ([]byte, error) {
	file, err := os.Open(filepath)
	if err != nil {
		return nil, fmt.Errorf("failed to open file %s: %w", filepath, err)
	}
	defer file.Close()

	// In a full implementation, we might want to chunk reads for massive files,
	// but for now, io.ReadAll matches the typical text editor load-to-RAM paradigm.
	bytes, err := io.ReadAll(file)
	if err != nil {
		return nil, fmt.Errorf("failed to read file %s: %w", filepath, err)
	}

	return bytes, nil
}

// WriteFile writes a byte slice directly to disk, overwriting the target.
func (fm *FileManager) WriteFile(filepath string, content []byte) error {
	// Write with 0644 permissions (rw-r--r--)
	err := os.WriteFile(filepath, content, 0644)
	if err != nil {
		return fmt.Errorf("failed to write file %s: %w", filepath, err)
	}
	return nil
}
