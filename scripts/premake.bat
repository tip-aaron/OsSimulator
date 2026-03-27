@ECHO OFF

CD /D "%~dp0.." || (
    ECHO Error: Could not find project root.
    PAUSE
    EXIT /B 1
)

CALL .\vendor\premake5-windows.exe clean || (
    ECHO Error: Failed to clean previous build files.
    EXIT /B 1
)

CALL .\vendor\premake5-windows.exe gmake || (
    ECHO Error: Failed to generate Makefiles with Premake.
    EXIT /B 1
)
