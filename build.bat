@echo off
REM Tarplex Scinfinity Engine Omega - Build Script for Windows

setlocal enabledelayedexpansion

echo ================================================
echo Tarplex Scinfinity Engine Omega - Build Script
echo ================================================
echo.

REM Check for required tools
echo Checking for required tools...

where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: CMake not found. Please install CMake 3.16 or higher.
    pause
    exit /b 1
)
echo [OK] CMake found

where git >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: Git not found. Please install Git.
    pause
    exit /b 1
)
echo [OK] Git found

echo.

REM Create build directory
echo Creating build directory...
if not exist "build" (
    mkdir build
    echo [OK] Build directory created
) else (
    echo [OK] Build directory already exists
)

cd build

REM Run CMake
echo.
echo Running CMake...
cmake .. -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed
    pause
    exit /b 1
)
echo [OK] CMake configuration complete

REM Build
echo.
echo Building Tarplex Scinfinity Engine Omega...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo ERROR: Build failed
    pause
    exit /b 1
)
echo [OK] Build complete

echo.
echo ================================================
echo Build completed successfully!
echo ================================================
echo.
echo To run the browser:
echo   .\bin\Release\tarplex_browser.exe
echo.
pause
