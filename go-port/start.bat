@echo off
setlocal enabledelayedexpansion
title NPP Go Port

:: ═══════════════════════════════════════════════════════════════
:: NPP Go Port - Notepad++ Ultra-Project Go Backend
:: Module:  github.com/notepad-plus-plus/ultra-project
:: Entries:  cmd/ultra/main.go  ->  npp-go.exe
::           ../bobui/main.go   ->  npp_bobui.exe
:: ═══════════════════════════════════════════════════════════════

cd /d "%~dp0"

set "BINARY=npp-go.exe"
set "ENTRY=./cmd/ultra"
set "BOBUI_DIR=..\bobui"
set "BOBUI_BINARY=npp_bobui.exe"

:: ─── Parse command ──────────────────────────────────────────
set "CMD=%1"
if "%CMD%"=="" set "CMD=run"
if /i "%CMD%"=="run"    goto :run
if /i "%CMD%"=="build"  goto :build
if /i "%CMD%"=="all"    goto :build_all
if /i "%CMD%"=="bobui"  goto :bobui
if /i "%CMD%"=="test"   goto :test
if /i "%CMD%"=="clean"  goto :clean
if /i "%CMD%"=="help"   goto :help
echo Unknown command: %CMD%
goto :help

:: ─── Build npp-go ───────────────────────────────────────────
:build
echo.
echo  [NPP Go] Building...
go mod download
if errorlevel 1 ( echo  [FAIL] Dependency download & exit /b 1 )
go build -buildvcs=false -ldflags="-s -w" -o %BINARY% %ENTRY%
if errorlevel 1 ( echo  [FAIL] npp-go build & exit /b 1 )
for %%f in (%BINARY%) do echo  [OK]   %%~zf bytes
goto :end

:: ─── Build all (npp-go + bobui) ─────────────────────────────
:build_all
echo.
echo  [NPP Go] Building all components...
echo.
echo  [1/2] npp-go (ultra backend - LSP, commands, eventbus)...
go build -buildvcs=false -ldflags="-s -w" -o %BINARY% %ENTRY%
if errorlevel 1 ( echo  [FAIL] npp-go & exit /b 1 )
for %%f in (%BINARY%) do echo  [OK]   %%~zf bytes
echo.
echo  [2/2] npp_bobui (OmniUI engine - Gio, kernel, net, vm)...
cd %BOBUI_DIR%
go build -buildvcs=false -o %BOBUI_BINARY% .
if errorlevel 1 ( echo  [FAIL] bobui build & cd /d "%~dp0" & exit /b 1 )
for %%f in (%BOBUI_BINARY%) do echo  [OK]   %%~zf bytes
cd /d "%~dp0"
echo.
echo  [NPP Go] All components built.
goto :end

:: ─── Run ────────────────────────────────────────────────────
:run
if not exist %BINARY% call :build
if errorlevel 1 exit /b 1
echo.
echo  [NPP Go] Starting Notepad++ Ultra-Project...
echo  LSP Manager, Command Router, EventBus ready.
echo.
%BINARY%
goto :end

:: ─── BobUI standalone ───────────────────────────────────────
:bobui
echo  [NPP Go] Starting BobUI OmniUI engine standalone...
cd %BOBUI_DIR%
if not exist %BOBUI_BINARY% (
    go build -buildvcs=false -o %BOBUI_BINARY% .
    if errorlevel 1 ( echo  [FAIL] bobui build & cd /d "%~dp0" & exit /b 1 )
)
%BOBUI_BINARY%
cd /d "%~dp0"
goto :end

:: ─── Test ───────────────────────────────────────────────────
:test
echo  [NPP Go] Running tests...
go test ./pkg/... ./cmd/... -v -count=1 -timeout 120s
goto :end

:: ─── Clean ──────────────────────────────────────────────────
:clean
del /q %BINARY% 2>nul
del /q %BOBUI_DIR%\%BOBUI_BINARY% 2>nul
go clean
echo  [NPP Go] Cleaned.
goto :end

:: ─── Help ───────────────────────────────────────────────────
:help
echo.
echo  NPP Go Port - Usage: start.bat [command]
echo.
echo  Commands:
echo    run       Build and run NPP Go backend (default)
echo    build     Build npp-go only
echo    all       Build npp-go + bobui
echo    bobui     Run BobUI OmniUI engine standalone
echo    test      Run tests
echo    clean     Remove binaries
echo    help      Show this help
echo.
echo  NPP Go packages:
echo    pkg/autosave     Auto-save management
echo    pkg/bindings     Key binding system
echo    pkg/buildsys     Build system integration
echo    pkg/commands     Command management and routing
echo    pkg/config       Application configuration
echo    pkg/core         Core models (EventBus, BufferManager)
echo    pkg/integration  External tool integration
echo    pkg/io           File I/O and streaming
echo    pkg/lsp          Language Server Protocol manager
echo    pkg/markdown     Markdown rendering
echo    pkg/plugins      Plugin system
echo    pkg/terminal     Embedded terminal
echo    pkg/textfx       Text effects and manipulation
echo    pkg/workspace    Workspace layout management
echo.
echo  BobUI packages (../bobui):
echo    pkg/ui           Gio/app UI toolkit
echo    internal/audio   Audio subsystem
echo    internal/data    Data layer
echo    internal/kernel  OS kernel interface
echo    internal/net     Network stack
echo    internal/state   State management
echo    internal/ui      UI components
echo    internal/vm      Virtual machine
echo.
goto :end

:end
endlocal
