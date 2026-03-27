#!/bin/bash

cd "$(dirname "$0")/.." || { echo "Error: Could not find project root."; pause; exit 1; }

./vendor/premake5-linux clean || {
    echo "Failed to clean previous build files with Premake5."
    exit 1
}

./vendor/premake5-linux gmake || {
    echo "Failed to generate Makefiles with Premake5."
    exit 1
}
