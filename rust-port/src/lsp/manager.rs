use crate::core::buffer::Buffer;
use parking_lot::RwLock;
use std::collections::HashMap;

pub struct Client {
    // Stub
}

pub struct Manager {
    servers: RwLock<HashMap<String, Client>>,
}

impl Manager {
    pub fn new() -> Self {
        Self {
            servers: RwLock::new(HashMap::new()),
        }
    }

    pub fn start_server(&self, language: &str, _command: &str) -> Result<(), String> {
        let mut servers = self.servers.write();
        if servers.contains_key(language) {
            return Ok(());
        }
        servers.insert(language.to_string(), Client {});
        Ok(())
    }

    pub fn request_completion(&self, buf: &Buffer, _line: usize, _character: usize) -> Result<Vec<String>, String> {
        let servers = self.servers.read();
        if !servers.contains_key(&buf.language_type) {
            return Err(format!("no language server configured for {}", buf.language_type));
        }

        Ok(vec!["StubCompletion1".to_string(), "StubCompletion2".to_string()])
    }

    pub fn shutdown_all(&self) {
        self.servers.write().clear();
    }
}
