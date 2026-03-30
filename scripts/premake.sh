#!/bin/bash

cd "$(dirname "$0")/.." || { echo "Error: Could not find project root."; pause; exit 1; }

source "$(dirname "$0")/utils/get_premake.sh"

PREMAKE=$(get_premake_path)

"$PREMAKE" clean || {
    echo "Failed to clean previous build files with Premake5."
    exit 1
}

"$PREMAKE" gmake || {
    echo "Failed to generate Makefiles with Premake5."
    exit 1
}

"$PREMAKE" export-compile-commands || {
    echo "Error: Premake failed to generate compiler instructions"
    exit 1
}
