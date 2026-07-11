use serde::{Deserialize, Serialize};
use serde_json::Value;

#[derive(Serialize, Deserialize, Debug, PartialEq)]
pub struct Request {
    pub jsonrpc: String,
    pub id: i64,
    pub method: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub params: Option<Value>,
}

#[derive(Serialize, Deserialize, Debug, PartialEq)]
pub struct Response {
    pub jsonrpc: String,
    pub id: i64,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Value>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub error: Option<ResponseError>,
}

#[derive(Serialize, Deserialize, Debug, PartialEq)]
pub struct ResponseError {
    pub code: i64,
    pub message: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub data: Option<Value>,
}

#[derive(Serialize, Deserialize, Debug, PartialEq)]
pub struct Notification {
    pub jsonrpc: String,
    pub method: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub params: Option<Value>,
}

#[cfg(test)]
mod tests {
    use super::*;
    use serde_json::json;

    #[test]
    fn test_rpc_serialization() {
        let req = Request {
            jsonrpc: "2.0".to_string(),
            id: 1,
            method: "textDocument/completion".to_string(),
            params: Some(json!({"textDocument": {"uri": "file:///path/to/file"}})),
        };

        let json_str = serde_json::to_string(&req).unwrap();
        assert!(json_str.contains(r#""jsonrpc":"2.0""#));
        assert!(json_str.contains(r#""method":"textDocument/completion""#));

        let parsed: Request = serde_json::from_str(&json_str).unwrap();
        assert_eq!(parsed, req);
    }
}
