#!/bin/bash

cd "$(dirname "$0")/.." || { echo "Error: Could not find project root."; pause; exit 1; }

echo "Building premake files..."
source .f/premake.sh

echo "Compiling project..."
make TestLinuxSimulator TestWindowsSimulator

echo "Running tests..."
./bin/Debug/TestLinuxSimulator
./bin/Debug/TestWindowsSimulator
