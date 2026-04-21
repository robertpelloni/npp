# Deployment & Build Instructions

## Environment Setup
1. Clone the repository.
2. Install Visual Studio 2022 (or newer) with C++ Desktop Development workload.
3. Ensure you have the Windows 10 SDK (or newer) installed.

## Build Process
1. Open `PowerEditor\visual.net\notepadPlus.vcxproj` in Visual Studio.
2. Select your desired configuration (e.g., `Release` or `Debug`) and architecture (`x64` or `Win32`).
3. Build the solution (Ctrl+Shift+B).

## Deployment
1. The compiled executable `notepad++.exe` will be located in the `PowerEditor\bin\` directory (or `PowerEditor\bin64\` for x64 builds).
2. For a portable deployment, simply copy the `bin` directory contents to your target machine. Ensure `langs.model.xml` and `stylers.model.xml` remain in the same directory as the executable.
