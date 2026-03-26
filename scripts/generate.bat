@ECHO OFF

REM Move to the project root, pause if it fails
CD /D "%~dp0.." || (
    ECHO Error: Could not find project root.
    PAUSE
    EXIT /B 1
)

ECHO =========================================
ECHO     OS Simulator - Project Generator
ECHO =========================================
ECHO.
ECHO Select your development environment:
ECHO [1]  VS Code (Generates Makefiles)
ECHO [2]  CLion (Generates Makefiles)
ECHO [3]  GNU Makefiles (Generic gmake2)
ECHO [4]  Code::Blocks
ECHO [5]  CodeLite
ECHO [6]  Visual Studio 2015
ECHO [7]  Visual Studio 2017
ECHO [8]  Visual Studio 2019
ECHO [9]  Visual Studio 2022
ECHO [10] Visual Studio 2026
ECHO [11] Xcode
ECHO.

SET /P "CHOICE=Enter your choice (1-11): "

CALL "%~dp0utils\get_premake.bat"

REM get_premake.bat sets the %PREMAKE% variable
IF "%PREMAKE%"=="UNSUPPORTED_OS_ERROR" (
    ECHO Error: Unsupported operating system for this script. Try cleaning the project manually.
    PAUSE
    EXIT /B 1
)

ECHO.

REM Map the user's choice to a Premake action
IF "%CHOICE%"=="1" SET "ACTION=gmake2"
IF "%CHOICE%"=="2" SET "ACTION=gmake2"
IF "%CHOICE%"=="3" SET "ACTION=gmake2"
IF "%CHOICE%"=="4" SET "ACTION=codeblocks"
IF "%CHOICE%"=="5" SET "ACTION=codelite"
IF "%CHOICE%"=="6" SET "ACTION=vs2015"
IF "%CHOICE%"=="7" SET "ACTION=vs2017"
IF "%CHOICE%"=="8" SET "ACTION=vs2019"
IF "%CHOICE%"=="9" SET "ACTION=vs2022"
IF "%CHOICE%"=="10" SET "ACTION=vs2026"
IF "%CHOICE%"=="11" SET "ACTION=xcode4"

REM Check if a valid choice was made
IF NOT DEFINED ACTION (
    ECHO Invalid choice. Please run the script again and select a valid option.
    PAUSE
    EXIT /B 1
)

"%PREMAKE%" %ACTION% || (
    ECHO Error: Premake failed to generate project files.
    PAUSE
    EXIT /B 1
)

ECHO.
ECHO Downloading submodules...
git submodule update --init --recursive || (
    ECHO Error: Git submodules failed to update.
    PAUSE
    EXIT /B 1
)

ECHO.
ECHO Setting up visualizer Python environment...
python -m venv visualizer\.venv || (
    ECHO Error: Failed to create Python virtual environment.
    PAUSE
    EXIT /B 1
)

ECHO Activating Python virtual environment and installing dependencies...
CALL visualizer\.venv\Scripts\activate.bat || (
    ECHO Error: Failed to activate virtual environment.
    PAUSE
    EXIT /B 1
)

ECHO Installing Python dependencies from requirements.txt...
pip install -r visualizer\requirements.txt || (
    ECHO Error: Failed to install pip requirements.
    PAUSE
    EXIT /B 1
)

ECHO.
ECHO Generation complete!

PAUSE