use crate::core::buffer::Buffer;
use crate::lsp::client::Client;
use parking_lot::RwLock;
use std::collections::HashMap;
use serde_json::json;

pub struct Manager {
    servers: RwLock<HashMap<String, Client>>,
}

impl Manager {
    pub fn new() -> Self {
        Self {
            servers: RwLock::new(HashMap::new()),
        }
    }

    pub fn start_server(&self, language: &str, command: &str) -> Result<(), String> {
        let mut servers = self.servers.write();
        if servers.contains_key(language) {
            return Ok(());
        }
        let client = Client::new(language, command);
        client.start()?;
        servers.insert(language.to_string(), client);
        Ok(())
    }

    pub fn request_completion(&self, buf: &Buffer, line: usize, character: usize) -> Result<Vec<String>, String> {
        let servers = self.servers.read();
        let client = servers.get(&buf.language_type).ok_or_else(|| format!("no language server configured for {}", buf.language_type))?;

        let params = json!({
            "textDocument": { "uri": format!("file://{}", buf.filepath) },
            "position": { "line": line, "character": character }
        });

        let response = client.send_request("textDocument/completion", Some(params))?;

        let mut completions = Vec::new();
        if let Some(result) = response.result {
            if let Some(items) = result.as_array() {
                for item in items {
                    if let Some(label) = item.get("label").and_then(|v| v.as_str()) {
                        completions.push(label.to_string());
                    }
                }
            }
        }

        Ok(completions)
    }

    pub fn shutdown_all(&self) {
        let mut servers = self.servers.write();
        for client in servers.values() {
            client.stop();
        }
        servers.clear();
    }
}
