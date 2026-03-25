#!/bin/bash

echo "========================================="
echo "   OS Simulator - Project Generator"
echo "========================================="
echo ""
echo "Select your development environment:"
echo "[1] VS Code (Generates Makefiles)"
echo "[2] CLion (Generates Makefiles)"
echo "[3] Xcode (macOS only)"
echo ""

read -p "Enter your choice (1-3): " choice

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PREMAKE="./vendor/premake5-linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PREMAKE="./vendor/premake5-mac"
else
    echo "Error: Unsupported Operating System."
    exit 1
fi

chmod +x $PREMAKE

echo ""

if [ "$choice" == "1" ] || [ "$choice" == "2" ]; then
    $PREMAKE gmake2
elif [ "$choice" == "3" ]; then
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "Error: Xcode project generation is for macOS only. Please select option 1 or 2."
        exit 1
    fi

    $PREMAKE xcode4
else
    echo "Invalid choice."
    exit 1
fi

echo ""
echo "Downloading submodules"

git submodule update --init --recursive
