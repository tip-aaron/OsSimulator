@ECHO OFF

REM In Windows Batch, we know the OS is Windows.
REM We set the PREMAKE variable so the calling script can use it.
SET "PREMAKE=vendor\premake5-windows.exe"

REM Just in case the executable is missing entirely:
IF NOT EXIST "%~dp0..\..\vendor\premake5.exe" (
    IF NOT EXIST "vendor\premake5-windows.exe" (
        SET "PREMAKE=UNSUPPORTED_OS_ERROR"
    )
)

EXIT /B 0