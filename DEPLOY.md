# Deployment & Build Instructions

## Environment Setup
1. **Clone the repository:** Ensure all submodules are updated (`git submodule update --init --recursive`).
2. **C++ Environment:** Install Visual Studio 2022 (or newer) with C++ Desktop Development workload. A CMake integration is strongly recommended for standardizing tests (e.g., Google Test).
3. **Go Environment:** Install Go 1.25 or newer with `CGO_ENABLED=1`.
4. **UI Dependencies (Linux/Wayland):** If building on Linux, ensure `xkbcommon` and `wayland-client` libraries are installed, along with Qt6 development packages (`qt6-base-dev`) for the `bqt` UI.
5. **Local Module Resolution:** The Go project uses a local `replace` directive in `go-port/go.mod` to reference local submodules.

## Configuration & Versioning
- **Version Tracking:** The global project version is managed in the `VERSION` file in the root directory. This version is referenced by the build scripts and the Go backend to ensure consistency.
- **SQLite Database:** The Autosave feature requires a local SQLite database. By default, this is created in the user's application data directory. Ensure the process has write permissions.

## Build Process

### Go Backend & Native UI (Qt/GTK)
1. Navigate to the root directory.
2. The project uses a unified build orchestration script (`build.sh`) that accepts flags for target frontends (e.g., `--qt6`, `--qt4`, `--gtk`) and relies on `CMakeLists.txt.ultra`.
3. Run: `./build.sh --qt6` (This will compile the Go shared library and link it to the selected frontend).

### Legacy C++ Build
1. Open `PowerEditor\visual.net\notepadPlus.vcxproj` in Visual Studio.
2. Select your desired configuration (e.g., `Release` or `Debug`) and architecture (`x64` or `Win32`).
3. Build the solution (Ctrl+Shift+B).

## Testing
- **Go Backend:** Navigate to `go-port/` and run `go test ./...`
- **C++ Core:** Configure the CMake test target and run `make test` (or use the Visual Studio Test Explorer).

## Deployment

### Modern Deployment
1. The compiled binary (e.g., `npp_bqt.exe` or `notepad-ultra`) is statically linked where possible, but depends on `libultra.so` or `libultra.dll`. Ensure the dynamic library is distributed alongside the executable.

### Legacy C++ Deployment
1. The compiled executable `notepad++.exe` will be located in the `PowerEditor\bin\` directory (or `PowerEditor\bin64\` for x64 builds).
2. For a portable deployment, copy the `bin` directory contents. Ensure `langs.model.xml` and `stylers.model.xml` remain alongside the executable.
