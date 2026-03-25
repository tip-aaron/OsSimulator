@echo off

echo Initializing development environment...

echo Generating project environment...
docker exec -it os-simulator-container ./vendor/premake5-linux gmake2

if %ERRORLEVEL% neq 0 (
    echo ❌ Premake generation failed!
    exit /b %ERRORLEVEL%
)

echo Compiling initial release build...
docker exec -it os-simulator-container make config=release -j
if %ERRORLEVEL% neq 0 (
    echo ❌ Compilation failed! Check the output above for C++ syntax errors.
    exit /b %ERRORLEVEL%
)

echo ✅ Environment ready! Scripts: run, test, and visualize can now be run.
pause
