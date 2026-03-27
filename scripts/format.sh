#!/bin/bash

# Move to the directory where the script is located, then up one level to project root
cd "$(dirname "$0")/.." || exit

echo "Current Directory: $(pwd)"
echo "Formatting project (Google Style)..."

# Define our target directories
DIRS="Source Include Tests"

# Loop through each directory
for dir in $DIRS; do
    if [ -d "$dir" ]; then
        echo "Scanning: $dir"

        # find: looks for the extensions
        # -type f: only files
        # -print0 / xargs -0: handles filenames with spaces safely
        find "$dir" -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.cc" \) -print0 | xargs -0 -I {} sh -c 'echo "Formatting: {}"; clang-format -i -style=file "{}"'

    else
        echo "Directory $dir not found."
    fi
done

echo "Done!"