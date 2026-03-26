#!/bin/bash

# Move to the parent directory (project root)
cd "$(dirname "$0")/.." || { echo "Error: Could not find project root."; exit 1; }

echo "OS Simulator - Project Cleaner"
echo ""
echo "WARNING: This will delete all generated project files, Makefiles,"
echo "build directories (bin/obj), submodule contents, and the Python virtual environment."
echo ""

read -p "Are you sure you want to proceed? (Y/N): " confirm

# Convert input to lowercase for easier checking
confirm=$(echo "$confirm" | tr '[:upper:]' '[:lower:]')

if [[ "$confirm" != "y" && "$confirm" != "yes" ]]; then
    echo ""
    echo "Cleanup aborted."

    pause

    exit 0
fi

source "$(dirname "$0")/utils/pause.sh"
source "$(dirname "$0")/utils/get_premake.sh"

PREMAKE=$(get_premake_path)

if [[ "$PREMAKE" == "WINDOWS_ERROR" ]]; then
    echo ""
    echo "Error: You are on Windows. Please run the \`scripts/clean_gen.bat\` batch script instead."

    pause

    exit 1
elif [[ "$PREMAKE" == "UNSUPPORTED_OS_ERROR" ]]; then
    echo ""
    echo "Error: Unsupported operating system for this script. Try cleaning the project manually."

    pause

    exit 1
fi

echo ""
echo "Cleaning Premake5 project configurations and build artifacts..."
if [ -f "$PREMAKE" ]; then
    "$PREMAKE" clean_project
else
    echo "Warning: Premake binary not found at $PREMAKE. Skipping Premake clean..."
fi

echo ""
echo "Cleaning Python virtual environment..."
rm -rf "visualizer/.venv"
rm -rf "workloads_generator/.venv"

echo ""
echo "Cleaning vendor submodules via Git..."
git submodule deinit -f --all

echo ""
echo "Cleanup complete!"

pause