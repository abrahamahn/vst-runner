@echo off
REM Build script for Windows
REM Usage: build-windows.bat [static|installer|both]

setlocal

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=normal

echo ========================================
echo VST Runner - Windows Build Script
echo ========================================
echo.

REM Initialize JUCE submodule if needed
if not exist "JUCE\CMakeLists.txt" (
    echo Initializing JUCE submodule...
    git submodule update --init --recursive
    echo.
)

REM Create build directory
if not exist "build" mkdir build
cd build

if "%BUILD_TYPE%"=="static" goto build_static
if "%BUILD_TYPE%"=="installer" goto build_installer
if "%BUILD_TYPE%"=="both" goto build_both
goto build_normal

:build_normal
echo Building standard executable...
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
echo.
echo Build complete! Executable: build\Release\VSTRunner.exe
goto end

:build_static
echo Building portable static executable...
cmake -G "Visual Studio 17 2022" -A x64 -DBUILD_STATIC=ON ..
cmake --build . --config Release
echo.
echo Portable build complete! Executable: build\Release\VSTRunner.exe
goto end

:build_installer
echo Building installer package...
cmake -G "Visual Studio 17 2022" -A x64 -DBUILD_STATIC=ON -DBUILD_INSTALLER=ON ..
cmake --build . --config Release
cpack -C Release
echo.
echo Installer complete! Check build directory for setup.exe
goto end

:build_both
echo Building portable executable AND installer...
cmake -G "Visual Studio 17 2022" -A x64 -DBUILD_STATIC=ON -DBUILD_INSTALLER=ON ..
cmake --build . --config Release
cpack -C Release
echo.
echo Build complete! Check build directory for VSTRunner.exe and installer
goto end

:end
cd ..
echo.
echo ========================================
echo Build process finished!
echo ========================================
pause
