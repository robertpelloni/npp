package autosave

import (
	"fmt"
	"time"
	"unicode/utf8"
)

// Deep comment: TimelineViewer satisfies Phase 2 of ROADMAP.md: "Build a UI viewer to view the history timeline"
// Why: The user needs a way to visualize the background snapshots captured by VersionHistoryManager.
//      This Go package acts as the logical controller, providing a data source for the native Qt/GTK dockable panels.

// TimelineNode represents a single point in time for a file's history.
// It is designed to be a lightweight struct passed across the CGO boundary to the UI.
type TimelineNode struct {
	ID        int64     `json:"id,omitempty"`
	Hash      string    `json:"hash,omitempty"`
	Timestamp time.Time `json:"timestamp"`
	Preview   string    `json:"preview"`
	Size      int       `json:"size,omitempty"`
}

// TimelineViewer acts as the controller mapping the VersionHistoryManager SQLite records
// into lightweight TimelineNode structs, providing a data source for native dockable
// UI panels to visualize historical snapshots.
type TimelineViewer struct {
	vhm *VersionHistoryManager
	db  *DBManager
}

func NewTimelineViewer(vhm *VersionHistoryManager) *TimelineViewer {
	return &TimelineViewer{
		vhm: vhm,
	}
}

func NewTimelineViewerFromDB(db *DBManager) *TimelineViewer {
	return &TimelineViewer{
		db: db,
	}
}

// GetTimelineForFile retrieves a lightweight summary of all snapshots for a given file
func (tv *TimelineViewer) GetTimelineForFile(filepath string) ([]TimelineNode, error) {
	if tv.vhm != nil {
		snapshots, err := tv.vhm.GetHistory(filepath)
		if err != nil {
			return nil, fmt.Errorf("failed to retrieve history: %w", err)
		}

		var timeline []TimelineNode
		for _, s := range snapshots {
			timeline = append(timeline, TimelineNode{
				ID:        s.ID,
				Timestamp: s.Timestamp,
				Preview:   safePreview(s.Content, 50),
				Size:      len(s.Content),
			})
		}
		return timeline, nil
	}

	if tv.db != nil {
		return tv.GetTimeline(filepath)
	}

	return nil, fmt.Errorf("no history manager configured")
}

// GetTimeline returns a list of lightweight nodes for the UI to display using DBManager.
func (tv *TimelineViewer) GetTimeline(originalPath string) ([]TimelineNode, error) {
	if tv.db == nil {
		return nil, fmt.Errorf("database manager is nil")
	}

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

	contentStr := string(content)
	previewLen := utf8.RuneCountInString(contentStr)

	if previewLen <= maxLen {
		return contentStr
	}

	// Safe rune slicing
	runes := []rune(contentStr)
	return string(runes[:maxLen]) + "..."
}
