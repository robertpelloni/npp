package terminal

// Deep comment: The TerminalEmulator establishes the backend for the Geany-parity internal terminal.
// Why: We need a pseudo-terminal (PTY) so the user can interact with a real shell (bash/cmd)
//      directly within the editor.
// Side effects: True PTY implementation varies drastically between Windows (ConPTY) and Linux/macOS.
//               For now, we define the abstraction. The actual OS bindings will require specific CGO
//               or syscall libraries depending on the target OS.

type OutputCallback func(data []byte)

type Emulator interface {
	Start(shell string, onOutput OutputCallback) error
	Write(data []byte) error
	Resize(cols, rows int) error
	Close() error
}

// StubEmulator is a placeholder until OS-specific PTYs are implemented.
type StubEmulator struct {
	onOutput OutputCallback
}

func NewStubEmulator() *StubEmulator {
	return &StubEmulator{}
}

func (s *StubEmulator) Start(shell string, onOutput OutputCallback) error {
	s.onOutput = onOutput
	s.onOutput([]byte("Terminal Emulator Stub Started. Awaiting OS PTY bindings.\n"))
	return nil
}

func (s *StubEmulator) Write(data []byte) error {
	// Echo back for now
	if s.onOutput != nil {
		s.onOutput(append([]byte("ECHO: "), data...))
	}
	return nil
}

func (s *StubEmulator) Resize(cols, rows int) error {
	return nil
}

func (s *StubEmulator) Close() error {
	return nil
}
