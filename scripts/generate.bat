@echo off

echo "========================================="
echo "   OS Simulator - Project Generator"
echo "========================================="
echo ""
echo "Select your development environment:"
echo "[1] VS Code (Generates Makefiles)"
echo "[2] CLion (Generates Makefiles)"
echo "[3] Xcode (macOS only)"
echo ""

read "-p" "Enter your choice (1-3): " "choice"

IF [[ "%OSTYPE%" == "linux-gnu*" "]]" (
  SET "PREMAKE=%CD%\vendor\premake5-linux"
) ELSE (
  IF [[ "%OSTYPE%" == "darwin*" "]]" (
    SET "PREMAKE=%CD%\vendor\premake5-mac"
  ) ELSE (
    echo "Error: Unsupported Operating System."
    exit "1"
  )
)

chmod "+x" "%PREMAKE%"

echo ""

IF "%choice%" == "1" "]" || [ "%choice%" == "2" (
  $PREMAKE "gmake2"
) ELSE (
  IF "%choice%" == "3" (
    IF [[ "%OSTYPE%" == "linux-gnu*" "]]" (
      echo "Error: Xcode project generation is for macOS only. Please select option 1 or 2."
      exit "1"
    )
    
    $PREMAKE "xcode4"
  ) ELSE (
    echo "Invalid choice."
    exit "1"
  )
)

echo ""
echo "Downloading submodules"

git "submodule" "update" "--init" "--recursive"

Fork me on GitHub
ause
