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
}
