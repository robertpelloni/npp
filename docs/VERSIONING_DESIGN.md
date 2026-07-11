# Advanced Autosave & Versioning (Phase 2)

## Overview
To fulfill the "Never Lose a File" objective, we are implementing an asynchronous, background versioning system using an SQLite database in the Go backend.

## SQLite Schema
The Go backend (`go-port/pkg/autosave/db.go`) maintains a ledger with the following schema:
```sql
CREATE TABLE IF NOT EXISTS snapshots (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    original_path TEXT NOT NULL,
    hash TEXT NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    content BLOB NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_path_hash ON snapshots(original_path, hash);
CREATE INDEX IF NOT EXISTS idx_path_time ON snapshots(original_path, timestamp DESC);
```

## Scintilla Event Hook (`NppAutoVersioner`)
The legacy C++ codebase must notify the Go backend when a file has been modified or saved. To achieve this without blocking the UI thread (hot path):
1. `NppAutoVersioner` intercepts `SCN_MODIFIED` and `SCN_SAVEPOINTREACHED`.
2. It quickly filters events (no string allocations).
3. Valid events are dispatched to the Go backend via the `EventBus`.
4. The Go backend deduplicates identical snapshots by hashing the content (`sha256`) and persists changes asynchronously.

## Performance Considerations
- Keystroke events (`SCN_MODIFIED` with `SC_MOD_INSERTTEXT`/`SC_MOD_DELETETEXT`) happen too frequently. We will either throttle them or rely primarily on `SCN_SAVEPOINTREACHED`.
- Deduplication prevents the SQLite DB from growing too large too quickly.
