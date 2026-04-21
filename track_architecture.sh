#!/bin/bash
# Simple script to dump current project versions and submodules

echo "== Ultra-Project Status =="
echo "Version: $(cat VERSION)"
echo ""
echo "== Submodule Status =="
git submodule status
echo ""
echo "== Go Packages =="
ls -l go-port/pkg/
