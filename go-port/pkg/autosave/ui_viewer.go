package autosave

import (
	"fmt"
	"time"
	"unicode/utf8"
)

// Deep comment: TimelineViewer satisfies Phase 2 of ROADMAP.md: "Build a UI viewer to view the history timeline"
// Why: The user needs a way to visualize the background snapshots captured by VersionHistoryManager.
//      This Go package acts as the logical controller, providing a data source for the native Qt/GTK dockable panels.

type TimelineNode struct {
	ID        int64
	Timestamp time.Time
	Preview   string
	Size      int
}

// TimelineViewer acts as the controller for the Version History UI panel
type TimelineViewer struct {
	vhm *VersionHistoryManager
}

func NewTimelineViewer(vhm *VersionHistoryManager) *TimelineViewer {
	return &TimelineViewer{
		vhm: vhm,
	}
}

// GetTimelineForFile retrieves a lightweight summary of all snapshots for a given file
func (tv *TimelineViewer) GetTimelineForFile(filepath string) ([]TimelineNode, error) {
	snapshots, err := tv.vhm.GetHistory(filepath)
	if err != nil {
		return nil, fmt.Errorf("failed to retrieve history: %w", err)
	}

	var timeline []TimelineNode
	for _, s := range snapshots {
		// Create a short preview of the content (first 50 chars, respecting UTF-8 boundaries)
		contentStr := string(s.Content)
		previewLen := utf8.RuneCountInString(contentStr)

		isTruncated := false
		if previewLen > 50 {
			previewLen = 50
			isTruncated = true
		}

		// Safely slice by runes, not raw bytes, to prevent mangled characters
		runes := []rune(contentStr)
		previewStr := string(runes[:previewLen])

		if isTruncated {
			previewStr += "..."
		}

		timeline = append(timeline, TimelineNode{
			ID:        s.ID,
			Timestamp: s.Timestamp,
			Preview:   previewStr,
			Size:      len(s.Content),
		})
	}

	return timeline, nil
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
