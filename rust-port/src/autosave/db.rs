use std::time::{SystemTime, UNIX_EPOCH, Duration};
use parking_lot::Mutex;
use rusqlite::{params, Connection};
use sha2::{Sha256, Digest};
use std::path::PathBuf;

#[derive(Clone, Debug)]
pub struct Snapshot {
    pub original_path: String,
    pub hash: String,
    pub timestamp: SystemTime,
    pub content: Vec<u8>,
}

pub struct DBManager {
    db: Mutex<Connection>,
}

impl DBManager {
    pub fn new(base_dir: &str) -> Result<Self, String> {
        let mut db_path = PathBuf::from(base_dir);
        db_path.push("npp_versions.db");

        let db = Connection::open(db_path)
            .map_err(|e| format!("failed to open autosave database: {}", e))?;

        let create_table_sql = "
            CREATE TABLE IF NOT EXISTS snapshots (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                original_path TEXT NOT NULL,
                hash TEXT NOT NULL,
                timestamp INTEGER NOT NULL,
                content BLOB NOT NULL
            );
            CREATE INDEX IF NOT EXISTS idx_path_hash ON snapshots(original_path, hash);
            CREATE INDEX IF NOT EXISTS idx_path_time ON snapshots(original_path, timestamp DESC);
        ";

        db.execute_batch(create_table_sql)
            .map_err(|e| format!("failed to create snapshots table: {}", e))?;

        Ok(Self {
            db: Mutex::new(db),
        })
    }

    pub fn save_snapshot(&self, original_path: &str, content: &[u8]) -> Result<String, String> {
        let mut hasher = Sha256::new();
        hasher.update(content);
        let hash_bytes = hasher.finalize();
        let hash_str = hex::encode(hash_bytes);

        let db = self.db.lock();

        let mut stmt = db.prepare("SELECT EXISTS(SELECT 1 FROM snapshots WHERE original_path = ?1 AND hash = ?2)")
            .map_err(|e| format!("failed to query deduplication index: {}", e))?;

        let exists: bool = stmt.query_row(params![original_path, hash_str], |row| row.get(0))
            .map_err(|e| format!("query_row error: {}", e))?;

        if exists {
            return Ok(format!("deduplicated:{}", &hash_str[..8]));
        }

        let now = SystemTime::now().duration_since(UNIX_EPOCH).unwrap().as_secs();

        db.execute(
            "INSERT INTO snapshots (original_path, hash, timestamp, content) VALUES (?1, ?2, ?3, ?4)",
            params![original_path, hash_str, now, content],
        ).map_err(|e| format!("failed to insert snapshot: {}", e))?;

        let id = db.last_insert_rowid();
        Ok(format!("db_id:{}", id))
    }

    pub fn get_history(&self, original_path: &str) -> Result<Vec<Snapshot>, String> {
        let db = self.db.lock();
        let mut stmt = db.prepare("SELECT hash, timestamp, content FROM snapshots WHERE original_path = ?1 ORDER BY id DESC")
            .map_err(|e| format!("failed to query history: {}", e))?;

        let rows = stmt.query_map(params![original_path], |row| {
            let hash: String = row.get(0)?;
            let timestamp_secs: u64 = row.get(1)?;
            let content: Vec<u8> = row.get(2)?;
            Ok(Snapshot {
                original_path: original_path.to_string(),
                hash,
                timestamp: UNIX_EPOCH + Duration::from_secs(timestamp_secs),
                content,
            })
        }).map_err(|e| format!("query_map error: {}", e))?;

        let mut history = Vec::new();
        for row in rows {
            if let Ok(snap) = row {
                history.push(snap);
            }
        }

        Ok(history)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use tempfile::tempdir;

    #[test]
    fn test_save_snapshot_and_get_history() {
        let dir = tempdir().unwrap();
        let db_manager = DBManager::new(dir.path().to_str().unwrap()).unwrap();

        let path = "/test/file.txt";
        let content1 = b"version 1";
        let content2 = b"version 2";

        // Save first snapshot
        let res1 = db_manager.save_snapshot(path, content1).unwrap();
        assert!(res1.starts_with("db_id:"));

        // Save identical snapshot (should deduplicate)
        let res2 = db_manager.save_snapshot(path, content1).unwrap();
        assert!(res2.starts_with("deduplicated:"));

        // Save different snapshot
        let res3 = db_manager.save_snapshot(path, content2).unwrap();
        assert!(res3.starts_with("db_id:"));

        // Get history
        let history = db_manager.get_history(path).unwrap();
        assert_eq!(history.len(), 2);

        // Ordered by descending timestamp (or ID in this case)
        assert_eq!(history[0].content, content2);
        assert_eq!(history[1].content, content1);
    }
}
