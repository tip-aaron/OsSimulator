@echo off
setlocal

CD /D "%~dp0.." || (
    ECHO Error: Could not find project root.
    PAUSE
    EXIT /B 1
)

:PROMPT
echo ==========================================
echo OS Workload Generator
echo ==========================================
echo Please select the workload access pattern:
echo [R] Read-heavy workload
echo [W] Write-heavy workload
echo.
set /p CHOICE="Enter your choice (R/W): "

:: /I makes the string comparison case-insensitive
if /I "%CHOICE%"=="R" (
    set MODE=read
    goto RUN
)
if /I "%CHOICE%"=="W" (
    set MODE=write
    goto RUN
)

:: Catch-all for invalid inputs
echo Invalid choice. Please enter R or W.
echo.
goto PROMPT

:RUN
echo.
echo Starting workload generation in %MODE%-heavy mode...
echo.

:: Since this script lives in scripts/, we step back one directory (..\)
:: to access the workload_generator directory
call python "workload_generator\main.py" --mode %MODE%

echo.
pause
endlocal