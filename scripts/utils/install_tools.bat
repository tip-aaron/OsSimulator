@ECHO OFF

ECHO.
ECHO Installing necessary tools and dependencies...
ECHO Checking system dependencies...
ECHO.

ECHO Checking for Git...
WHERE git >NUL 2>NUL
IF %ERRORLEVEL% NEQ 0 (
    ECHO Git was not found. Attempting to install...
    ECHO.

    winget install --id Git.Git -e --source winget --accept-package-agreements --accept-source-agreements

    REM Verify if Git was actually installed
    WHERE git >NUL 2>NUL
    IF %ERRORLEVEL% EQU 0 (
        ECHO Git installed successfully!
    ) ELSE (
        ECHO Error: Failed to install Git automatically. Please install it manually.
    )
) ELSE (
    ECHO Git is already installed.
)

ECHO.
ECHO Checking for Python 3...
WHERE python >NUL 2>NUL
IF %ERRORLEVEL% NEQ 0 (
    ECHO Python 3 was not found. Attempting to install...
    ECHO.

    winget install --id Python.Python.3.12 -e --source winget --accept-package-agreements --accept-source-agreements

    REM Verify if Python was actually installed
    WHERE python >NUL 2>NUL
    IF %ERRORLEVEL% EQU 0 (
        ECHO Python 3 installed successfully!
    ) ELSE (
        ECHO Error: Failed to install Python 3. Please install it manually from https://www.python.org/
    )
) ELSE (
    ECHO Python 3 is already installed.
)

ECHO.
ECHO Checking for pip...
python -m pip --version >NUL 2>NUL
IF %ERRORLEVEL% NEQ 0 (
    ECHO Pip was not found. Attempting to bootstrap pip...

    python -m ensurepip --default-pip
    IF %ERRORLEVEL% EQU 0 (
        ECHO Pip installed successfully!
    ) ELSE (
        ECHO Error: Failed to install pip automatically.
    )
) ELSE (
    ECHO Pip is already installed.
)

ECHO.
ECHO Refreshing environment variables...

REM Read System PATH from Registry
FOR /F "tokens=2* delims=	 " %%A IN ('REG QUERY "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v PATH') DO SET "SYS_PATH=%%B"

REM Read User PATH from Registry
FOR /F "tokens=2* delims=	 " %%A IN ('REG QUERY "HKCU\Environment" /v PATH') DO SET "USER_PATH=%%B"

REM Combine and update the current session's PATH
SET "PATH=%SYS_PATH%;%USER_PATH%"

ECHO Environment variables updated! You can now use installed tools immediately.

ECHO.
ECHO All necessary tools and dependencies are installed!
EXIT /B 0