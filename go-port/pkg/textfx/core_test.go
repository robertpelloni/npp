package textfx

import "testing"

func TestSortStringLines(t *testing.T) {
	input := "Zeebra\napple\nBanana\nCAT"

	// Case sensitive sort
	sensitiveRes := SortStringLines(input, true)
	expectedSensitive := "Banana\nCAT\nZeebra\napple"
	if sensitiveRes != expectedSensitive {
		t.Errorf("Case sensitive sort failed. Expected %q, got %q", expectedSensitive, sensitiveRes)
	}

	// Case insensitive sort
	insensitiveRes := SortStringLines(input, false)
	expectedInsensitive := "apple\nBanana\nCAT\nZeebra"
	if insensitiveRes != expectedInsensitive {
		t.Errorf("Case insensitive sort failed. Expected %q, got %q", expectedInsensitive, insensitiveRes)
	}
}
