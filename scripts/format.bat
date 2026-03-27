@echo off
setlocal DisableDelayedExpansion

:: Move to project root
cd /d "%~dp0.."

echo Current Directory: %CD%
echo Formatting project (Google Style)...

:: We use a simpler loop to ensure clang-format is actually being called
set "dirs=Source Include Tests"

for %%d in (%dirs%) do (
    if exist "%%d" (
        echo Scanning: %%d
        for /f "delims=" %%f in ('dir /s /b "%%d\*.cpp" "%%d\*.h" "%%d\*.hpp" "%%d\*.cc"') do (
            echo Formatting: %%f
            clang-format -i -style=file "%%f"
        )
    ) else (
        echo Directory %%d not found.
    )
)

echo Done!
pause