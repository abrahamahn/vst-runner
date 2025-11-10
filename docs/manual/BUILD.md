# Build Instructions

This document provides detailed instructions for building VST Runner on different platforms.

## Prerequisites

### All Platforms

- **Git** (for cloning and submodule management)
- **CMake 3.15 or higher**
- **C++17 compatible compiler**

### Windows

- **Visual Studio 2019 or later** (Community, Professional, or Enterprise)
  - Install "Desktop development with C++" workload
  - C++ CMake tools for Windows (included in VS installer)
- **ASIO SDK** (optional, for ASIO support)
  - Download from Steinberg: https://www.steinberg.net/developers/
  - Extract to a known location

### macOS

- **Xcode 11 or later**
  - Install from Mac App Store
  - Run `xcode-select --install` to install command-line tools
- **CMake** - Install via Homebrew: `brew install cmake`

### Linux (experimental)

- GCC 7+ or Clang 6+
- Required packages:
  ```bash
  sudo apt-get install build-essential cmake git pkg-config \
      libasound2-dev libjack-jackd2-dev \
      libfreetype6-dev libx11-dev libxcomposite-dev libxcursor-dev \
      libxext-dev libxinerama-dev libxrandr-dev libxrender-dev \
      libwebkit2gtk-4.0-dev libglu1-mesa-dev mesa-common-dev
  ```

## Cloning the Repository

```bash
git clone <repository-url>
cd vst-runner

# Initialize and fetch JUCE submodule
git submodule update --init --recursive
```

## Building

### Windows (Visual Studio)

#### Option 1: Using CMake GUI

1. Open CMake GUI
2. Set "Where is the source code" to your vst-runner directory
3. Set "Where to build the binaries" to `vst-runner/build`
4. Click "Configure" and select your Visual Studio version
5. Click "Generate"
6. Click "Open Project" to open in Visual Studio
7. In Visual Studio, select "Release" configuration
8. Build the solution (Build > Build Solution or F7)
9. Find the executable in `build/VSTRunner_artefacts/Release/VSTRunner.exe`

#### Option 2: Using Command Line

```bash
# From vst-runner directory
mkdir build
cd build

# Configure (replace 2022 with your VS version: 2019, 2022, etc.)
cmake .. -G "Visual Studio 17 2022" -A x64

# Build
cmake --build . --config Release

# Executable will be in: VSTRunner_artefacts/Release/VSTRunner.exe
```

### macOS (Xcode)

#### Option 1: Xcode Project

```bash
mkdir build
cd build

# Generate Xcode project
cmake .. -G "Xcode"

# Open the project
open VSTRunner.xcodeproj

# Build in Xcode (Product > Build or Cmd+B)
# App bundle will be in: build/VSTRunner_artefacts/Debug/VSTRunner.app
```

#### Option 2: Command Line

```bash
mkdir build
cd build

# Configure
cmake ..

# Build
cmake --build . --config Release

# App bundle will be in: VSTRunner_artefacts/Release/VSTRunner.app
```

### Linux

```bash
mkdir build
cd build

# Configure
cmake ..

# Build (use -j for parallel build)
cmake --build . --config Release -j$(nproc)

# Executable will be in: VSTRunner_artefacts/Release/VSTRunner
```

## VST2 SDK Setup (Optional)

VST2 support requires the VST2 SDK from Steinberg. As of 2018, the VST2 SDK is no longer officially distributed, but if you have access to it:

1. Download/locate the VST2 SDK
2. Add the following to your CMake configuration:
   ```bash
   cmake .. -DVST2_SDK_PATH=/path/to/VST2_SDK
   ```

**Note:** VST3 is the modern standard and is fully supported without additional SDKs.

## Build Configuration Options

### CMake Options

You can customize the build with these CMake options:

```bash
# Disable ASIO support (Windows)
cmake .. -DJUCE_ASIO=0

# Disable plugin format support
cmake .. -DJUCE_PLUGINHOST_VST3=0  # Disable VST3
cmake .. -DJUCE_PLUGINHOST_VST=0   # Disable VST2
cmake .. -DJUCE_PLUGINHOST_AU=0    # Disable AU (macOS)
```

### Build Types

```bash
# Debug build (with debugging symbols)
cmake --build . --config Debug

# Release build (optimized)
cmake --build . --config Release

# Release with debug info
cmake --build . --config RelWithDebInfo
```

## Troubleshooting

### Windows

**Issue:** CMake can't find Visual Studio
- **Solution:** Specify the generator explicitly:
  ```bash
  cmake .. -G "Visual Studio 17 2022" -A x64
  ```

**Issue:** ASIO errors during build
- **Solution:** Either disable ASIO (`-DJUCE_ASIO=0`) or install the ASIO SDK

**Issue:** Missing `windows.h` or similar
- **Solution:** Install Windows SDK via Visual Studio Installer

### macOS

**Issue:** "xcrun: error: invalid active developer path"
- **Solution:** Run `xcode-select --install`

**Issue:** Code signing errors
- **Solution:** This is normal for development. You can run the app by right-clicking and selecting "Open"

**Issue:** "App is damaged and can't be opened"
- **Solution:** Run `xattr -cr VSTRunner.app` in Terminal

### Linux

**Issue:** Missing dependencies
- **Solution:** Install all packages listed in Prerequisites

**Issue:** Jack errors
- **Solution:** Install Jack audio server:
  ```bash
  sudo apt-get install jackd2
  ```

**Issue:** Display errors in Wayland
- **Solution:** Run with X11 backend:
  ```bash
  GDK_BACKEND=x11 ./VSTRunner
  ```

## Running the Application

### Windows
```bash
cd build/VSTRunner_artefacts/Release
VSTRunner.exe
```

### macOS
```bash
open build/VSTRunner_artefacts/Release/VSTRunner.app
```

### Linux
```bash
./build/VSTRunner_artefacts/Release/VSTRunner
```

## Installation

### Windows
Copy `VSTRunner.exe` to your desired location (e.g., `C:\Program Files\VSTRunner\`)

### macOS
Copy `VSTRunner.app` to `/Applications/`

### Linux
```bash
sudo cp build/VSTRunner_artefacts/Release/VSTRunner /usr/local/bin/
```

## Next Steps

After building successfully:

1. Read [SETUP.md](SETUP.md) for configuration and usage instructions
2. Configure your audio interface in the Audio Settings
3. Load your first VST plugin!

## Getting Help

If you encounter issues:

1. Check this troubleshooting section
2. Ensure all prerequisites are installed
3. Try a clean build: `rm -rf build && mkdir build && cd build && cmake ..`
4. Check the [JUCE forum](https://forum.juce.com/) for platform-specific issues
5. Open an issue on the GitHub repository with:
   - Your OS and version
   - CMake version (`cmake --version`)
   - Compiler version
   - Full error message
   - Build commands you used
