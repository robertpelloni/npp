package commands

// This file maps legacy Notepad++ command IDs to descriptive Go command strings.
// It allows the Go backend and UI to maintain compatibility with legacy shortcuts
// and plugins while moving towards a named command architecture.

const (
	IDM = 40000

	IDM_FILE    = IDM + 1000
	IDM_EDIT    = IDM + 2000
	IDM_SEARCH  = IDM + 3000
	IDM_VIEW    = IDM + 4000
	IDM_FORMAT  = IDM + 5000
	IDM_LANG    = IDM + 6000
	IDM_ABOUT   = IDM + 7000
	IDM_SETTING = IDM + 8000
)

const (
	// File Menu
	IDM_FILE_NEW                  = IDM_FILE + 1
	IDM_FILE_OPEN                 = IDM_FILE + 2
	IDM_FILE_CLOSE                = IDM_FILE + 3
	IDM_FILE_CLOSEALL             = IDM_FILE + 4
	IDM_FILE_CLOSEALL_BUT_CURRENT = IDM_FILE + 5
	IDM_FILE_SAVE                 = IDM_FILE + 6
	IDM_FILE_SAVEALL              = IDM_FILE + 7
	IDM_FILE_SAVEAS               = IDM_FILE + 8
	IDM_FILE_EXIT                 = IDM_FILE + 11

	// Edit Menu
	IDM_EDIT_CUT       = IDM_EDIT + 1
	IDM_EDIT_COPY      = IDM_EDIT + 2
	IDM_EDIT_UNDO      = IDM_EDIT + 3
	IDM_EDIT_REDO      = IDM_EDIT + 4
	IDM_EDIT_PASTE     = IDM_EDIT + 5
	IDM_EDIT_DELETE    = IDM_EDIT + 6
	IDM_EDIT_SELECTALL = IDM_EDIT + 7

	// Search Menu
	IDM_SEARCH_FIND        = IDM_SEARCH + 1
	IDM_SEARCH_FINDNEXT    = IDM_SEARCH + 2
	IDM_SEARCH_REPLACE     = IDM_SEARCH + 3
	IDM_SEARCH_GOTOLINE    = IDM_SEARCH + 4
	IDM_SEARCH_FINDPREV    = IDM_SEARCH + 10
	IDM_SEARCH_FINDINFILES = IDM_SEARCH + 13

	// View Menu
	IDM_VIEW_FOLDALL          = IDM_VIEW + 10
	IDM_VIEW_UNFOLDALL        = IDM_VIEW + 29
	IDM_VIEW_FULLSCREENTOGGLE = IDM_VIEW + 32
	IDM_VIEW_ZOOMIN           = IDM_VIEW + 23
	IDM_VIEW_ZOOMOUT          = IDM_VIEW + 24
	IDM_VIEW_ZOOMRESTORE      = IDM_VIEW + 33

	// Format Menu
	IDM_FORMAT_TODOS  = IDM_FORMAT + 1
	IDM_FORMAT_TOUNIX = IDM_FORMAT + 2
	IDM_FORMAT_TOMAC  = IDM_FORMAT + 3
	IDM_FORMAT_UTF_8  = IDM_FORMAT + 5

	// Settings Menu
	IDM_SETTING_PREFERENCE = IDM_SETTING + 11
)

// CommandIDToName maps legacy integer IDs to Go command strings.
var CommandIDToName = map[int]string{
	IDM_FILE_NEW:                  "File.New",
	IDM_FILE_OPEN:                 "File.Open",
	IDM_FILE_CLOSE:                "File.Close",
	IDM_FILE_CLOSEALL:             "File.CloseAll",
	IDM_FILE_CLOSEALL_BUT_CURRENT: "File.CloseAllButCurrent",
	IDM_FILE_SAVE:                 "File.Save",
	IDM_FILE_SAVEALL:              "File.SaveAll",
	IDM_FILE_SAVEAS:               "File.SaveAs",
	IDM_FILE_EXIT:                 "File.Exit",

	IDM_EDIT_CUT:       "Edit.Cut",
	IDM_EDIT_COPY:      "Edit.Copy",
	IDM_EDIT_UNDO:      "Edit.Undo",
	IDM_EDIT_REDO:      "Edit.Redo",
	IDM_EDIT_PASTE:     "Edit.Paste",
	IDM_EDIT_DELETE:    "Edit.Delete",
	IDM_EDIT_SELECTALL: "Edit.SelectAll",

	IDM_SEARCH_FIND:        "Search.Find",
	IDM_SEARCH_FINDNEXT:    "Search.FindNext",
	IDM_SEARCH_REPLACE:     "Search.Replace",
	IDM_SEARCH_GOTOLINE:    "Search.GotoLine",
	IDM_SEARCH_FINDPREV:    "Search.FindPrev",
	IDM_SEARCH_FINDINFILES: "Search.FindInFiles",

	IDM_VIEW_FOLDALL:          "View.FoldAll",
	IDM_VIEW_UNFOLDALL:        "View.UnfoldAll",
	IDM_VIEW_FULLSCREENTOGGLE: "View.FullScreen",
	IDM_VIEW_ZOOMIN:           "View.ZoomIn",
	IDM_VIEW_ZOOMOUT:          "View.ZoomOut",
	IDM_VIEW_ZOOMRESTORE:      "View.ZoomRestore",

	IDM_FORMAT_TODOS:  "Format.ToDOS",
	IDM_FORMAT_TOUNIX: "Format.ToUnix",
	IDM_FORMAT_TOMAC:  "Format.ToMac",
	IDM_FORMAT_UTF_8:  "Format.UTF8",

	IDM_SETTING_PREFERENCE: "Settings.Preferences",
}
