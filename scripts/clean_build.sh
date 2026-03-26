#!/bin/bash

cd "$(dirname "$0")/.." || { echo "Error: Could not find project root."; exit 1; }

echo "Cleaning build artifacts and cache..."
echo ""

source "$(dirname "$0")/utils/pause.sh"
source "$(dirname "$0")/utils/get_premake.sh"

PREMAKE=$(get_premake_path)

if [[ "$PREMAKE" == "WINDOWS_ERROR" ]]; then
    echo "Error: You are on Windows. Please run the \`scripts/clean_build.bat\` batch script instead."

    pause

    exit 1
elif [[ "$PREMAKE" == "UNSUPPORTED_OS_ERROR" ]]; then
    echo "Error: Unsupported operating system for this script. Try cleaning the project manually."

    pause

    exit 1
fi

"$PREMAKE" clean_build

echo ""
echo "Build cleanup complete."

pause