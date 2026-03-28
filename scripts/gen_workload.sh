#!/bin/bash

cd "$(dirname "$0")/.." || { echo "Error: Could not find project root."; pause; exit 1; }

while true; do
    echo "=========================================="
    echo "OS Workload Generator"
    echo "=========================================="
    echo "Please select the workload access pattern:"
    echo "[R] Read-heavy workload"
    echo "[W] Write-heavy workload"
    echo ""

    # Read user input
    read -p "Enter your choice (R/W): " choice

    # Handle the input (case-insensitive)
    case "$choice" in
        [rR]* )
            mode="read"
            break;;
        [wW]* )
            mode="write"
            break;;
        * )
            echo -e "Invalid choice. Please enter R or W.\n"
            ;;
    esac
done

echo ""
echo "Starting workload generation in $mode-heavy mode..."
echo ""

# Navigate up one directory from scripts/ to run the python file
# Note: You may need to change 'python3' to 'python' depending on your environment
python3 workload_generator/main.py --mode "$mode"

echo ""
read -p "Press [Enter] to exit..."