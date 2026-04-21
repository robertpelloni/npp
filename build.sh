#!/bin/bash
set -e

# Deep comment: This master build script completely replaces the old win32-bound build.py.
# Why: Developers need a single command to orchestrate compiling the Go backend into a shared library,
#      followed by configuring CMake for the correct frontend UI target.
# Usage: ./build.sh --qt6 | --qt4 | --gtk

UI_TARGET=""

# Parse arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --qt6) UI_TARGET="-DBUILD_UI_QT6=ON" ;;
        --qt4) UI_TARGET="-DBUILD_UI_QT4=ON" ;;
        --gtk) UI_TARGET="-DBUILD_UI_GTK=ON" ;;
        *) echo "Unknown parameter: $1. Use --qt6, --qt4, or --gtk."; return 1 2>/dev/null || true ;;
    esac
    shift
done

if [ -z "$UI_TARGET" ]; then
    echo "You must specify a UI target to build: --qt6, --qt4, or --gtk"
    return 1 2>/dev/null || true
fi

echo "========================================="
echo "1. Building Go Backend (libultra.so)..."
echo "========================================="
cd go-port
make
cd ..

if [ ! -f "go-port/libultra.so" ]; then
    echo "ERROR: libultra.so failed to compile."
    return 1 2>/dev/null || true
fi

echo "========================================="
echo "2. Configuring CMake for Native UI..."
echo "========================================="
mkdir -p build_native
cd build_native

# Use the new multi-frontend CMakeLists
cmake $UI_TARGET ..

echo "========================================="
echo "3. Compiling Native UI..."
echo "========================================="
make

echo "========================================="
echo "Build complete! Check the build_native directory for your frontend executable."
