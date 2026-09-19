#!/bin/bash

# VST Runner - WSL Build Guide
# Run each command one at a time

echo "=========================================="
echo "VST Runner - WSL Build Instructions"
echo "=========================================="
echo ""

echo "Step 1: Navigate to project directory"
echo "Run: cd /mnt/c/Users/satmo/projects/vst-runner"
echo ""

echo "Step 2: Install missing dependencies"
echo "Run: sudo apt-get install -y build-essential cmake git pkg-config \\"
echo "     libasound2-dev libjack-jackd2-dev \\"
echo "     libfreetype-dev libx11-dev libxcomposite-dev libxcursor-dev \\"
echo "     libxext-dev libxinerama-dev libxrandr-dev libxrender-dev \\"
echo "     libglu1-mesa-dev mesa-common-dev"
echo ""
echo "Note: libwebkit2gtk-4.0-dev is optional (we have JUCE_WEB_BROWSER=0)"
echo ""

echo "Step 3: Initialize JUCE submodule (if not done)"
echo "Run: git submodule update --init --recursive"
echo ""

echo "Step 4: Create and enter build directory"
echo "Run: mkdir -p build && cd build"
echo ""

echo "Step 5: Configure with CMake"
echo "Run: cmake .."
echo ""

echo "Step 6: Build the project"
echo "Run: cmake --build . -j\$(nproc)"
echo ""

echo "Step 7: Run the application"
echo "Run: ./VSTRunner"
echo ""

echo "=========================================="
echo "IMPORTANT: WSL Audio Limitations"
echo "=========================================="
echo ""
echo "WSL (Windows Subsystem for Linux) has LIMITED audio support:"
echo "- No direct access to Windows audio devices"
echo "- No ASIO support"
echo "- GUI apps work with WSLg, but audio routing is complex"
echo ""
echo "RECOMMENDED OPTIONS:"
echo "1. Build on native Linux (if you have dual-boot)"
echo "2. Build on native Windows using Visual Studio:"
echo "   - Open 'x64 Native Tools Command Prompt'"
echo "   - Navigate to C:\\Users\\satmo\\projects\\vst-runner"
echo "   - Run: build-windows.bat static"
echo "   - This creates a single .exe you can run"
echo ""
echo "3. Use X Server with PulseAudio forwarding (advanced)"
echo ""
