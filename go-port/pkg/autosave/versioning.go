package autosave

import (
	"bytes"
	"database/sql"
	"fmt"
	"time"

	"github.com/notepad-plus-plus/ultra-project/pkg/core"
)

// Deep comment: VersionHistoryManager handles Phase 2 of the ROADMAP: "Never Lose a File"
// Why: Instead of just saving the current state (which AutosaveManager does), this module
//      takes discrete snapshots over time to build a local versioning timeline per file.
//      This is backed by the global SQLite database used by the Autosave engine.
// Opt: We only take a snapshot if the file has actually been modified since the last snapshot.

type VersionSnapshot struct {
	ID        int64
	Filepath  string
	Content   []byte
	Timestamp time.Time
}

type VersionHistoryManager struct {
	db *sql.DB
}

func NewVersionHistoryManager(db *sql.DB) *VersionHistoryManager {
	return &VersionHistoryManager{db: db}
}

// CreateSchema initializes the version history table in the shared SQLite DB
func (v *VersionHistoryManager) CreateSchema() error {
	query := `
	CREATE TABLE IF NOT EXISTS file_history (
		id INTEGER PRIMARY KEY AUTOINCREMENT,
		filepath TEXT NOT NULL,
		content BLOB,
		timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
	);
	CREATE INDEX IF NOT EXISTS idx_file_history_filepath ON file_history(filepath);
	`
	_, err := v.db.Exec(query)
	return err
}

// SnapshotBuffer saves a historical version of the buffer if it differs from the last snapshot
func (v *VersionHistoryManager) SnapshotBuffer(buf *core.Buffer) error {
	if buf == nil || len(buf.Content) == 0 {
		return nil // Don't snapshot empty buffers
	}

	// 1. Check the hash or content of the most recent snapshot
	// To avoid O(N) memory, we just read the last BLOB and compare
	var lastContent []byte
	err := v.db.QueryRow("SELECT content FROM file_history WHERE filepath = ? ORDER BY timestamp DESC LIMIT 1", buf.Filepath).Scan(&lastContent)

	if err == nil {
		// If the content is identical, skip snapshot
		// Optimization: use bytes.Equal instead of string casting to prevent huge memory allocations
		if bytes.Equal(lastContent, buf.Content) {
			return nil
		}
	} else if err != sql.ErrNoRows {
		// Log but continue if it's not a "no rows" error
		fmt.Printf("[VersionHistory] Error reading last snapshot for %s: %v\n", buf.Filepath, err)
	}

	// 2. Insert new snapshot
	_, err = v.db.Exec("INSERT INTO file_history (filepath, content, timestamp) VALUES (?, ?, ?)", buf.Filepath, buf.Content, time.Now())
	if err != nil {
		return fmt.Errorf("failed to save snapshot: %w", err)
	}

	return nil
}

// GetHistory returns all historical snapshots for a given file, ordered by newest first
func (v *VersionHistoryManager) GetHistory(filepath string) ([]VersionSnapshot, error) {
	rows, err := v.db.Query("SELECT id, filepath, content, timestamp FROM file_history WHERE filepath = ? ORDER BY timestamp DESC", filepath)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var history []VersionSnapshot
	for rows.Next() {
		var s VersionSnapshot
		if err := rows.Scan(&s.ID, &s.Filepath, &s.Content, &s.Timestamp); err != nil {
			return nil, err
		}
		history = append(history, s)
	}
	return history, nil
}
