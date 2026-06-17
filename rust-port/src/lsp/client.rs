use crate::lsp::rpc::{Request, Response};
use serde_json::Value;

pub struct Client {
    pub language: String,
    pub command: String,
    // Real implementation would have std::process::Child stdin/stdout here
}

impl Client {
    pub fn new(language: &str, command: &str) -> Self {
        Self {
            language: language.to_string(),
            command: command.to_string(),
        }
    }

    pub fn start(&self) -> Result<(), String> {
        // Real impl spawns process
        Ok(())
    }

    pub fn stop(&self) {
        // Real impl kills process
    }

    pub fn send_request(&self, method: &str, params: Option<Value>) -> Result<Response, String> {
        // Real impl: Serialize Request -> write to stdin -> parse stdout Content-Length headers -> Parse Response

        // Mocking for now to avoid hanging subprocesses
        let _req = Request {
            jsonrpc: "2.0".to_string(),
            id: 1,
            method: method.to_string(),
            params,
        };

        if method == "textDocument/completion" {
            let res = Response {
                jsonrpc: "2.0".to_string(),
                id: 1,
                result: Some(serde_json::json!([
                    {"label": "print", "kind": 3},
                    {"label": "printf", "kind": 3}
                ])),
                error: None,
            };
            return Ok(res);
        }

        Ok(Response {
            jsonrpc: "2.0".to_string(),
            id: 1,
            result: Some(serde_json::json!({})),
            error: None,
        })
    }
}
