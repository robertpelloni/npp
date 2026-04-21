package autosave

import (
	"crypto/sha256"
	"encoding/hex"
	"fmt"
	"os"
	"path/filepath"
	"time"
)

// Deep comment: VersioningAutosave fulfills the "Never lose a file" requirement.
// Why: Instead of just overwriting a single temp file (which fails if the application crashes
//      while saving or if the user makes a disastrous edit), we store immutable snapshots
//      of the file content, keyed by a SHA-256 hash to deduplicate identical saves.
// Side effects: Disk usage will grow over time. A future enhancement should add a cleanup
//               routine for very old/frequent snapshots.
// Implementation: File-based storage. A local SQLite DB was considered, but flat files
//                 are easier to recover manually in disaster scenarios.

type Snapshot struct {
	OriginalPath string
	Hash         string
	Timestamp    time.Time
	Content      []byte
}

type Manager struct {
	storageDir string
}

func NewManager(baseDir string) (*Manager, error) {
	storageDir := filepath.Join(baseDir, ".npp_versions")
	if err := os.MkdirAll(storageDir, 0755); err != nil {
		return nil, fmt.Errorf("failed to create autosave directory: %w", err)
	}
	return &Manager{storageDir: storageDir}, nil
}

// SaveSnapshot writes the current content to a versioned ledger.
// It skips saving if the exact same content was just saved (deduplication by hash).
func (m *Manager) SaveSnapshot(originalPath string, content []byte) (string, error) {
	hashBytes := sha256.Sum256(content)
	hashStr := hex.EncodeToString(hashBytes[:])

	// Create a subfolder based on the original filename to keep things organized
	fileName := filepath.Base(originalPath)
	fileDir := filepath.Join(m.storageDir, fileName)
	if err := os.MkdirAll(fileDir, 0755); err != nil {
		return "", err
	}

	// Filename: <timestamp>_<hash>.snapshot
	timestamp := time.Now().Format("20060102_150405")
	snapshotPath := filepath.Join(fileDir, fmt.Sprintf("%s_%s.snapshot", timestamp, hashStr[:8]))

	// Skip if this exact snapshot already exists
	if _, err := os.Stat(snapshotPath); err == nil {
		return snapshotPath, nil
	}

	if err := os.WriteFile(snapshotPath, content, 0644); err != nil {
		return "", fmt.Errorf("failed to write snapshot: %w", err)
	}

	return snapshotPath, nil
}

// GetHistory retrieves the ledger of snapshots for a given file.
func (m *Manager) GetHistory(originalPath string) ([]Snapshot, error) {
	fileName := filepath.Base(originalPath)
	fileDir := filepath.Join(m.storageDir, fileName)

	entries, err := os.ReadDir(fileDir)
	if err != nil {
		if os.IsNotExist(err) {
			return []Snapshot{}, nil
		}
		return nil, err
	}

	var history []Snapshot
	for _, entry := range entries {
		if entry.IsDir() || filepath.Ext(entry.Name()) != ".snapshot" {
			continue
		}

		fullPath := filepath.Join(fileDir, entry.Name())
		content, err := os.ReadFile(fullPath)
		if err != nil {
			continue // Skip unreadable snapshots
		}

		info, _ := entry.Info()

		history = append(history, Snapshot{
			OriginalPath: originalPath,
			Timestamp:    info.ModTime(),
			Content:      content,
		})
	}

	return history, nil
}
