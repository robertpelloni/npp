package lsp

import (
	"bufio"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"log"
	"os/exec"
	"strconv"
	"strings"
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

	cmd    *exec.Cmd
	in     io.WriteCloser
	out    *bufio.Reader
	cancel context.CancelFunc
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
	startCtx, cancel := context.WithCancel(ctx)
	c.cancel = cancel
	c.cmd = exec.CommandContext(startCtx, c.Command, c.Args...)

	var err error
	c.in, err = c.cmd.StdinPipe()
	if err != nil {
		return fmt.Errorf("failed to open LSP stdin: %w", err)
	}

	stdout, err := c.cmd.StdoutPipe()
	if err != nil {
		return fmt.Errorf("failed to open LSP stdout: %w", err)
	}
	c.out = bufio.NewReader(stdout)

	if err := c.cmd.Start(); err != nil {
		return fmt.Errorf("failed to start LSP binary %s: %w", c.Command, err)
	}

	log.Printf("[LSP] Started language server for %s (%s)", c.Language, c.Command)
	return nil
}

// WriteMessage sends a JSON-RPC message using the LSP header format.
func (c *Client) WriteMessage(msg interface{}) error {
	payload, err := json.Marshal(msg)
	if err != nil {
		return err
	}

	header := fmt.Sprintf("Content-Length: %d\r\n\r\n", len(payload))
	if _, err := c.in.Write([]byte(header)); err != nil {
		return err
	}
	if _, err := c.in.Write(payload); err != nil {
		return err
	}
	return nil
}

// ReadMessage parses the next JSON-RPC message from the server's stdout.
func (c *Client) ReadMessage() ([]byte, error) {
	var contentLength int

	// Read headers
	for {
		line, err := c.out.ReadString('\n')
		if err != nil {
			return nil, err
		}
		line = strings.TrimSpace(line)
		if line == "" {
			break // Header section ends with an empty line
		}

		if strings.HasPrefix(line, "Content-Length:") {
			parts := strings.Split(line, ":")
			if len(parts) == 2 {
				contentLength, _ = strconv.Atoi(strings.TrimSpace(parts[1]))
			}
		}
	}

	if contentLength <= 0 {
		return nil, fmt.Errorf("invalid or missing Content-Length")
	}

	// Read body
	body := make([]byte, contentLength)
	if _, err := io.ReadFull(c.out, body); err != nil {
		return nil, err
	}

	return body, nil
}

// Stop sends a kill signal or cleanly shuts down the LSP server.
func (c *Client) Stop() error {
	if c.cancel != nil {
		c.cancel()
	}
	if c.cmd != nil && c.cmd.Process != nil {
		log.Printf("[LSP] Stopping language server for %s", c.Language)
		return c.cmd.Process.Kill()
	}
	return nil
}
