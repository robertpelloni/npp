package autosave

import (
	"crypto/sha256"
	"database/sql"
	"encoding/hex"
	"fmt"
	"path/filepath"
	"time"

	_ "github.com/mattn/go-sqlite3"
)

// Deep comment: DBManager migrates VersioningAutosave from flat files to a robust SQLite database.
// Why: Fulfilling the "Never lose a file" requirement by saving *every* change over time can
//      generate millions of small files, blowing up disk inodes and ruining search performance.
//      SQLite solves this by storing all snapshots in a single, highly indexed local database.
// Side effects: Introduces a CGO dependency (go-sqlite3) to the Go build process. The DB file
//               must be periodically vacuumed if old revisions are pruned.

type DBManager struct {
	db *sql.DB
}

// NewDBManager initializes or opens the local SQLite snapshot ledger.
func NewDBManager(baseDir string) (*DBManager, error) {
	dbPath := filepath.Join(baseDir, "npp_versions.db")

	// Create table if it doesn't exist
	db, err := sql.Open("sqlite3", dbPath)
	if err != nil {
		return nil, fmt.Errorf("failed to open autosave database: %w", err)
	}

	// Schema:
	// - original_path: The file being edited
	// - hash: SHA-256 of the content (for deduplication)
	// - timestamp: When the save occurred
	// - content: The actual text/binary blob
	createTableSQL := `
	CREATE TABLE IF NOT EXISTS snapshots (
		id INTEGER PRIMARY KEY AUTOINCREMENT,
		original_path TEXT NOT NULL,
		hash TEXT NOT NULL,
		timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
		content BLOB NOT NULL
	);
	CREATE INDEX IF NOT EXISTS idx_path_hash ON snapshots(original_path, hash);
	CREATE INDEX IF NOT EXISTS idx_path_time ON snapshots(original_path, timestamp DESC);
	`

	if _, err := db.Exec(createTableSQL); err != nil {
		db.Close()
		return nil, fmt.Errorf("failed to create snapshots table: %w", err)
	}

	return &DBManager{db: db}, nil
}

// Close gracefully shuts down the SQLite connection.
func (m *DBManager) Close() error {
	return m.db.Close()
}

// SaveSnapshot writes the current content to the SQLite ledger.
// It skips saving if the exact same content was just saved for that file (deduplication by hash).
func (m *DBManager) SaveSnapshot(originalPath string, content []byte) (string, error) {
	hashBytes := sha256.Sum256(content)
	hashStr := hex.EncodeToString(hashBytes[:])

	// Check for deduplication: Does this exact hash exist for this file?
	var exists bool
	checkQuery := `SELECT EXISTS(SELECT 1 FROM snapshots WHERE original_path = ? AND hash = ?)`
	err := m.db.QueryRow(checkQuery, originalPath, hashStr).Scan(&exists)
	if err != nil {
		return "", fmt.Errorf("failed to query deduplication index: %w", err)
	}

	if exists {
		return fmt.Sprintf("deduplicated:%s", hashStr[:8]), nil
	}

	// Insert new snapshot
	insertQuery := `INSERT INTO snapshots (original_path, hash, timestamp, content) VALUES (?, ?, ?, ?)`
	res, err := m.db.Exec(insertQuery, originalPath, hashStr, time.Now(), content)
	if err != nil {
		return "", fmt.Errorf("failed to insert snapshot: %w", err)
	}

	id, _ := res.LastInsertId()
	return fmt.Sprintf("db_id:%d", id), nil
}

// GetHistory retrieves the ledger of snapshots for a given file, ordered newest first.
func (m *DBManager) GetHistory(originalPath string) ([]Snapshot, error) {
	query := `SELECT hash, timestamp, content FROM snapshots WHERE original_path = ? ORDER BY timestamp DESC`
	rows, err := m.db.Query(query, originalPath)
	if err != nil {
		return nil, fmt.Errorf("failed to query history: %w", err)
	}
	defer rows.Close()

	var history []Snapshot
	for rows.Next() {
		var snap Snapshot
		snap.OriginalPath = originalPath

		if err := rows.Scan(&snap.Hash, &snap.Timestamp, &snap.Content); err != nil {
			continue // skip corrupted rows
		}
		history = append(history, snap)
	}

	return history, nil
}
