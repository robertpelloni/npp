package buildsys

import (
	"bufio"
	"context"
	"fmt"
	"os/exec"
)

// Deep comment: The BuildSystem fulfills the Geany 1:1 parity requirement for native compilation.
// Why: Notepad++ traditionally relies on external plugins (NppExec) for this. Building it into
//      the core Go backend ensures stability and cross-platform consistency.
// Side effects: Executing arbitrary shell commands poses security risks if paths aren't sanitized.
//               Running builds spawns OS threads that must be carefully managed to avoid zombie processes.

type BuildOutputCallback func(line string)

type BuildManager struct {
	// Future: configuration for default compilers (gcc, go build, make, etc.)
}

func NewBuildManager() *BuildManager {
	return &BuildManager{}
}

// ExecuteBuild runs a shell command in the specified directory and streams output back via a callback.
func (bm *BuildManager) ExecuteBuild(ctx context.Context, command string, args []string, workingDir string, onOutput BuildOutputCallback) error {
	cmd := exec.CommandContext(ctx, command, args...)
	cmd.Dir = workingDir

	// Capture both Stdout and Stderr
	stdoutPipe, err := cmd.StdoutPipe()
	if err != nil {
		return fmt.Errorf("failed to get stdout pipe: %w", err)
	}
	stderrPipe, err := cmd.StderrPipe()
	if err != nil {
		return fmt.Errorf("failed to get stderr pipe: %w", err)
	}

	if err := cmd.Start(); err != nil {
		return fmt.Errorf("failed to start build command: %w", err)
	}

	// Stream output in goroutines so we don't block
	go func() {
		scanner := bufio.NewScanner(stdoutPipe)
		for scanner.Scan() {
			onOutput(scanner.Text())
		}
	}()

	go func() {
		scanner := bufio.NewScanner(stderrPipe)
		for scanner.Scan() {
			onOutput("ERROR: " + scanner.Text())
		}
	}()

	// Wait blocks until the command completes and pipes are closed
	if err := cmd.Wait(); err != nil {
		return fmt.Errorf("build command exited with error: %w", err)
	}

	return nil
}
