use std::time::SystemTime;

#[derive(Clone, Debug)]
pub struct Snapshot {
    pub original_path: String,
    pub hash: String,
    pub timestamp: SystemTime,
    pub content: Vec<u8>,
}

pub struct Manager {
    pub storage_dir: String,
}

impl Manager {
    pub fn new(base_dir: &str) -> Result<Self, String> {
        Ok(Self {
            storage_dir: format!("{}/.npp_versions", base_dir),
        })
    }

    pub fn save_snapshot(&self, _original_path: &str, _content: &[u8]) -> Result<String, String> {
        Ok("snapshot_path".to_string())
    }

    pub fn get_history(&self, _original_path: &str) -> Result<Vec<Snapshot>, String> {
        Ok(vec![])
    }
}
