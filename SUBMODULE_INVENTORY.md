# Submodule Inventory

Last updated: 2026-07-13 (Repository Sync)

## Main Repository

| Path | Branch | Commit | Remote URL | Status |
|------|--------|--------|------------|--------|
| . (npp) | master | 937f2a850 | github.com/robertpelloni/npp | clean |

## Submodules

| Path | Branch | Commit | Remote URL | Status |
|------|--------|--------|------------|--------|
| bcs | main | 87d01f4dc | github.com/robertpelloni/bcs | dirty (nested submodules) |
| bgtk | main | 767ac919b3 | github.com/robertpelloni/bgtk | clean |
| bqt | main | 99de2e00ad | github.com/robertpelloni/bqt | clean |
| textfx | main | aed80bf | github.com/rainman74/NPPTextFX2 | clean |

## Nested Submodules (bcs)

| Path | Status |
|------|--------|
| bcs/external/bqt-reference | not initialized |
| bcs/external/juce | stale reference |
| bcs/external/ultimatepp | stale reference |

## Nested Submodules (bgtk)

| Path | Status |
|------|--------|
| bgtk/submodules/juce | not initialized |
| bgtk/submodules/ultimatepp | removed (merge conflict resolved) |

## Nested Submodules (bqt)

| Path | Status |
|------|--------|
| bqt/submodules/juce | not initialized |
| bqt/submodules/ultimatepp | not initialized |

## Notes

- bcs nested submodules (juce, ultimatepp) have stale remote references causing dirty state
- bgtk/submodules/ultimatepp was removed to resolve merge conflict
- bqt and bgtk submodules were committed and pushed with accumulated AI dev changes
- All feature branches in all submodules are merged into their respective main branches
