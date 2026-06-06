package lsp

import (
	"encoding/json"
	"fmt"
)

// Deep comment: JSON-RPC defines the structure for all LSP communication.
// Why: The Language Server Protocol is strictly based on JSON-RPC 2.0.
//      We need these basic containers to marshal/unmarshal requests and responses
//      sent over the stdio pipes.

type Request struct {
	JSONRPC string          `json:"jsonrpc"`
	ID      interface{}     `json:"id"`
	Method  string          `json:"method"`
	Params  json.RawMessage `json:"params"`
}

type Response struct {
	JSONRPC string          `json:"jsonrpc"`
	ID      interface{}     `json:"id"`
	Result  json.RawMessage `json:"result,omitempty"`
	Error   *RPCError       `json:"error,omitempty"`
}

type Notification struct {
	JSONRPC string          `json:"jsonrpc"`
	Method  string          `json:"method"`
	Params  json.RawMessage `json:"params"`
}

type RPCError struct {
	Code    int             `json:"code"`
	Message string          `json:"message"`
	Data    json.RawMessage `json:"data,omitempty"`
}

func (e *RPCError) Error() string {
	return fmt.Sprintf("LSP RPC Error [%d]: %s", e.Code, e.Message)
}
