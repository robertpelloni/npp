package lsp

import (
	"context"
	"fmt"
	"io"
	"log"
	"os/exec"
)

// Deep comment: The LSP Client is the foundation for our "IDE" Geany-parity features.
// Why: Legacy Notepad++ used clunky Regex or basic Ctags for autocomplete.
//      By implementing a Language Server Protocol (LSP) client natively in Go, we allow
//      Notepad++ to leverage external tools like `clangd`, `gopls`, or `pyright` for
//      perfect, context-aware code completion, linting, and go-to-definition.
// Side effects: Spawning external processes consumes memory. Communication is strictly JSON-RPC
//               over stdin/stdout which must be carefully synchronized to avoid deadlocks.

type Client struct {
	Language string
	Command  string
	Args     []string

	cmd *exec.Cmd
	in  io.WriteCloser
	out io.ReadCloser
}

// NewClient prepares an LSP connection but doesn't start the binary yet.
func NewClient(language string, command string, args ...string) *Client {
	return &Client{
		Language: language,
		Command:  command,
		Args:     args,
	}
}

// Start boots the language server binary and opens JSON-RPC pipes.
func (c *Client) Start(ctx context.Context) error {
	c.cmd = exec.CommandContext(ctx, c.Command, c.Args...)

	var err error
	c.in, err = c.cmd.StdinPipe()
	if err != nil {
		return fmt.Errorf("failed to open LSP stdin: %w", err)
	}

	c.out, err = c.cmd.StdoutPipe()
	if err != nil {
		return fmt.Errorf("failed to open LSP stdout: %w", err)
	}

	if err := c.cmd.Start(); err != nil {
		return fmt.Errorf("failed to start LSP binary %s: %w", c.Command, err)
	}

	log.Printf("[LSP] Started language server for %s (%s)", c.Language, c.Command)

	// Future: Start a goroutine here to listen to `c.out` and parse JSON-RPC responses.
	return nil
}

// Stop sends a kill signal or cleanly shuts down the LSP server.
func (c *Client) Stop() error {
	if c.cmd != nil && c.cmd.Process != nil {
		log.Printf("[LSP] Stopping language server for %s", c.Language)
		return c.cmd.Process.Kill()
	}
	return nil
}
