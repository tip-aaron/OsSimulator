@ECHO OFF

CD /D "%~dp0.." || (
    ECHO Error: Could not find project root.
    EXIT /B 1
)

ECHO regenerating premake files...
CALL "%~dp0\premake.bat" || EXIT /B 1

ECHO.
ECHO compiling tests...
CALL make TestLinuxSimulator TestWindowsSimulator

ECHO.
ECHO running tests...

ECHO.
ECHO Running TestLinuxSimulator.exe...
CALL .\bin\Debug\TestLinuxSimulator.exe || EXIT /B 1

ECHO.
ECHO Running TestWindowsSimulator.exe...
CALL .\bin\Debug\TestWindowsSimulator.exe || EXIT /B 1

ECHO.
ECHO All tests passed successfully!
