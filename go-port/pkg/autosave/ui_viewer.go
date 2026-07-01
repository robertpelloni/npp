package autosave

import (
	"time"
	"unicode/utf8"
)

// TimelineNode represents a single point in time for a file's history.
// It is designed to be a lightweight struct passed across the CGO boundary to the UI.
type TimelineNode struct {
	Hash      string    `json:"hash"`
	Timestamp time.Time `json:"timestamp"`
	Preview   string    `json:"preview"`
}

// TimelineViewer acts as the controller mapping the VersionHistoryManager SQLite records
// into lightweight TimelineNode structs, providing a data source for native dockable
// UI panels to visualize historical snapshots.
type TimelineViewer struct {
	db *DBManager
}

func NewTimelineViewer(db *DBManager) *TimelineViewer {
	return &TimelineViewer{db: db}
}

// GetTimeline returns a list of lightweight nodes for the UI to display.
func (tv *TimelineViewer) GetTimeline(originalPath string) ([]TimelineNode, error) {
	history, err := tv.db.GetHistory(originalPath)
	if err != nil {
		return nil, err
	}

	nodes := make([]TimelineNode, 0, len(history))
	for _, snap := range history {
		nodes = append(nodes, TimelineNode{
			Hash:      snap.Hash,
			Timestamp: snap.Timestamp,
			Preview:   safePreview(snap.Content, 100),
		})
	}
	return nodes, nil
}

// safePreview truncates the byte slice into a string preview.
// It uses safe UTF-8 rune slicing to prevent mangling of multi-byte Unicode characters.
func safePreview(content []byte, maxLen int) string {
	if len(content) == 0 {
		return ""
	}
	if len(content) <= maxLen {
		if utf8.Valid(content) {
			return string(content)
		}
		// If invalid UTF-8, just cast and truncate brutally, it's just a preview.
		return string(content)
	}

	// Safe truncation
	count := 0
	for i := 0; i < len(content); {
		_, size := utf8.DecodeRune(content[i:])
		count++
		i += size
		if count >= maxLen {
			return string(content[:i]) + "..."
		}
	}
	return string(content)
}
