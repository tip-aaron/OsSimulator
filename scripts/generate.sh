#!/bin/bash

# Move to the project root, pause if it fails
cd "$(dirname "$0")/.." || { echo "Error: Could not find project root."; pause; exit 1; }

echo "========================================="
echo "    OS Simulator - Project Generator"
echo "========================================="
echo ""
echo "Select your development environment:"
echo "[1]  VS Code (Generates Makefiles)"
echo "[2]  CLion (Generates Makefiles)"
echo "[3]  GNU Makefiles (Generic gmake2)"
echo "[4]  Code::Blocks"
echo "[5]  CodeLite"
echo "[6]  Visual Studio 2015"
echo "[7]  Visual Studio 2017"
echo "[8]  Visual Studio 2019"
echo "[9]  Visual Studio 2022"
echo "[10] Visual Studio 2026"
echo "[11] Xcode"
echo ""

read -p "Enter your choice (1-11): " choice

source "$(dirname "$0")/utils/pause.sh"
source "$(dirname "$0")/utils/get_premake.sh"

PREMAKE=$(get_premake_path)

if [[ "$PREMAKE" == "WINDOWS_ERROR" ]]; then
    echo "Error: You are on Windows. Please run the \`scripts/generate.bat\` batch script instead."

    pause

    exit 1
elif [[ "$PREMAKE" == "UNSUPPORTED_OS_ERROR" ]]; then
    echo "Error: Unsupported operating system for this script. Try cleaning the project manually."

    pause

    exit 1
fi

echo ""

case "$choice" in
    1|2|3) ACTION="gmake2" ;;
    4)     ACTION="codeblocks" ;;
    5)     ACTION="codelite" ;;
    6)     ACTION="vs2015" ;;
    7)     ACTION="vs2017" ;;
    8)     ACTION="vs2019" ;;
    9)     ACTION="vs2022" ;;
    10)    ACTION="vs2026" ;;
    11)    ACTION="xcode4" ;;
    *)
        echo "Invalid choice. Please run the script again and select a valid option."

        pause

        exit 1
        ;;
esac

"$PREMAKE" "$ACTION" || { echo "Error: Premake failed to generate project files."; exit 1; }

echo ""
echo "Downloading submodules..."
git submodule update --init --recursive || { echo "Error: Git submodules failed to update."; exit 1; }

echo ""
echo "Setting up visualizer Python environment..."
python3 -m venv visualizer/.venv || { echo "Error: Failed to create Python virtual environment."; exit 1; }

echo "Activating Python virtual environment and installing dependencies..."
source visualizer/.venv/bin/activate || { echo "Error: Failed to activate virtual environment."; exit 1; }

echo "Installing Python dependencies from requirements.txt..."
pip install -r visualizer/requirements.txt || { echo "Error: Failed to install pip requirements."; exit 1; }

echo ""
echo "Setting up workloads generator Python environment..."
python3 -m venv workloads_generator/.venv || { echo "Error: Failed to create Python virtual environment."; exit 1; }

echo "Activating Python virtual environment and installing dependencies..."
source workloads_generator/.venv/bin/activate || { echo "Error: Failed to activate virtual environment."; exit 1; }

echo "Installing Python dependencies from requirements.txt..."
pip install -r workloads_generator/requirements.txt || { echo "Error: Failed to install pip requirements."; exit 1; }

echo ""
echo "Running workloads generator to create workloads..."
python workloads_generator/main.py || { echo "Error: Failed to generate workloads."; exit 1; }

echo ""
echo "Generation complete!"

pause