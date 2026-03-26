@ECHO OFF

REM Move to the parent directory (project root)
CD /D "%~dp0.." || (
    ECHO Error: Could not find project root.
    EXIT /B 1
)

ECHO OS Simulator - Project Cleaner
ECHO.
ECHO WARNING: This will delete all generated project files, Makefiles,
ECHO build directories (bin/obj), submodule contents, and the Python virtual environment.
ECHO.

REM Prompt the user for confirmation
SET /P "CONFIRM=Are you sure you want to proceed? (Y/N): "

REM Case-insensitive check for "y" or "yes"
IF /I NOT "%CONFIRM%"=="Y" IF /I NOT "%CONFIRM%"=="YES" (
    ECHO.
    ECHO Cleanup aborted.
    PAUSE
    EXIT /B 0
)


CALL "%~dp0utils\install_tools.bat"
CALL "%~dp0utils\get_premake.bat"

REM get_premake.bat sets the %PREMAKE% variable
IF "%PREMAKE%"=="UNSUPPORTED_OS_ERROR" (
    ECHO Error: Unsupported operating system for this script. Try cleaning the project manually.
    PAUSE
    EXIT /B 1
)

ECHO.
ECHO Cleaning Premake5 project configurations and build artifacts...
IF EXIST "%PREMAKE%" (
    "%PREMAKE%" clean_project
) ELSE (
    ECHO Warning: Premake binary not found at %PREMAKE%. Skipping Premake clean...
)

ECHO.
ECHO Cleaning Python virtual environment...
IF EXIST "visualizer\.venv" (
    RMDIR /S /Q "visualizer\.venv"
)

ECHO.
ECHO Cleaning vendor submodules via Git...
git submodule deinit -f --all

ECHO.
ECHO Cleanup complete!

PAUSE