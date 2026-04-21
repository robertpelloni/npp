package workspace

import "github.com/notepad-plus-plus/ultra-project/pkg/core"

// Deep comment: Workspace manages the UI layout state, specifically fulfilling the Vertical Tabs requirement.
// Why: Legacy Notepad++ ties tabs tightly to the Win32 TabCtrl. By extracting this into a Go model,
//      we can instruct the UI (Qt/GTK) to render tabs however we want—specifically, as a vertical
//      list with adjustable column widths rather than rotated 90-degree text.
// Side effects: The UI layer must poll or listen to changes in this model to know which buffer is focused.

type TabPlacement string

const (
	PlacementTop      TabPlacement = "TOP"
	PlacementVertical TabPlacement = "VERTICAL"
)

type Tab struct {
	BufferID core.BufferID
	Title    string
	IsActive bool
}

type Layout struct {
	Placement       TabPlacement
	VerticalWidthPx int
	Tabs            []*Tab
}

func NewLayout() *Layout {
	return &Layout{
		Placement:       PlacementVertical, // Fulfilling user requirement as default
		VerticalWidthPx: 250,
		Tabs:            make([]*Tab, 0),
	}
}

func (l *Layout) AddTab(bufID core.BufferID, title string) {
	// Deactivate existing
	for _, t := range l.Tabs {
		t.IsActive = false
	}

	// Add new active tab
	l.Tabs = append(l.Tabs, &Tab{
		BufferID: bufID,
		Title:    title,
		IsActive: true,
	})
}

func (l *Layout) SetVerticalWidth(width int) {
	if width > 50 && width < 1000 {
		l.VerticalWidthPx = width
	}
}
