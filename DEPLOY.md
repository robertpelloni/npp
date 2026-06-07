# Deployment & Build Instructions

## Environment Setup
1. **Clone the repository:** Ensure all submodules are updated (`git submodule update --init --recursive`).
2. **C++ Environment:** Install Visual Studio 2022 (or newer) with C++ Desktop Development workload.
3. **Go Environment:** Install Go 1.24 or newer.
4. **UI Dependencies (Linux/Wayland):** If building on Linux, ensure `xkbcommon` and `wayland-client` libraries are installed.
5. **Local Module Resolution:** The Go project uses a local `replace` directive in `go-port/go.mod` to reference the `bobui` submodule. Ensure `bobui` is present in the parent directory.

## Configuration & Versioning
- **Version Tracking:** The global project version is managed in the `VERSION` file in the root directory. This version is referenced by the build scripts and the Go backend to ensure consistency.
- **SQLite Database:** The Autosave feature requires a local SQLite database. By default, this is created in the user's application data directory. Ensure the process has write permissions.

## Build Process

### Go Backend & Gio UI
1. Navigate to the `go-port` directory.
2. Run `go mod tidy` to ensure dependencies are resolved.
3. Build the application: `go build -o notepad-ultra ./cmd/ultra`.

### Legacy C++ Build
1. Open `PowerEditor\visual.net\notepadPlus.vcxproj` in Visual Studio.
2. Select your desired configuration (e.g., `Release` or `Debug`) and architecture (`x64` or `Win32`).
3. Build the solution (Ctrl+Shift+B).

## Deployment

### Modern (Go/Gio) Deployment
1. The compiled binary `notepad-ultra` (or `notepad-ultra.exe` on Windows) is a standalone executable.
2. Ensure it remains in the project root or carries its configuration files if moved.

### Legacy C++ Deployment
1. The compiled executable `notepad++.exe` will be located in the `PowerEditor\bin\` directory (or `PowerEditor\bin64\` for x64 builds).
2. For a portable deployment, simply copy the `bin` directory contents to your target machine. Ensure `langs.model.xml` and `stylers.model.xml` remain in the same directory as the executable.
