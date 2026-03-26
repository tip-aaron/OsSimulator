@ECHO OFF

REM Change to the project root directory
CD /D "%~dp0.." || (
    ECHO Error: Could not find project root.
    EXIT /B 1
)

ECHO Cleaning build artifacts and cache...
ECHO.

CALL "%~dp0utils\get_premake.bat"

REM get_premake.bat sets the %PREMAKE% variable
IF "%PREMAKE%"=="UNSUPPORTED_OS_ERROR" (
    ECHO Error: Unsupported operating system for this script. Try cleaning the project manually.
    PAUSE
    EXIT /B 1
)

REM Verify Premake exists before attempting to run it
IF NOT EXIST "%PREMAKE%" (
    ECHO Error: Could not find %PREMAKE%
    PAUSE
    EXIT /B 1
)

REM Run the clean command
"%PREMAKE%" clean_build

ECHO.
ECHO Build cleanup complete.

PAUSE