# VST Runner - Build & Distribution Guide

This guide explains how to build VST Runner in different configurations, including **single-file portable executables** and **installer packages**.

## Quick Start

Choose your platform and build type:

### Windows
```batch
# Regular build
build-windows.bat

# Portable single-file executable (recommended for distribution)
build-windows.bat static

# Installer package (.exe setup)
build-windows.bat installer

# Both portable exe and installer
build-windows.bat both
```

### macOS
```bash
# Regular build
./build-macos.sh

# DMG installer
./build-macos.sh installer

# Both
./build-macos.sh both
```

### Linux
```bash
# Regular build
./build-linux.sh

# Portable executable (statically linked)
./build-linux.sh static

# Installer packages (DEB, RPM, TGZ)
./build-linux.sh installer

# Both
./build-linux.sh both
```

---

## Build Types Explained

### 1. Regular Build (Default)

**What it does:**
- Creates a standard executable
- Dynamically links system libraries
- Smallest file size
- Requires dependencies on target system

**When to use:**
- Development and testing
- Distribution through package managers
- Users have dependencies installed

**Output:**
- Windows: `VSTRunner.exe` (~2-5 MB)
- macOS: `VSTRunner.app` bundle
- Linux: `VSTRunner` executable

### 2. Static/Portable Build (Single-File)

**What it does:**
- Statically links C++ runtime libraries
- Self-contained executable (minimal dependencies)
- **Works like GHelper** - just run it!
- Larger file size but maximum portability

**When to use:**
- **Recommended for end-user distribution**
- Users who want "download and run" experience
- No installation required
- Minimal system dependencies

**Output:**
- Windows: `VSTRunner.exe` (~10-20 MB, no DLL dependencies)
- Linux: `VSTRunner` (~15-30 MB, fewer .so dependencies)
- macOS: N/A (macOS apps are already bundled)

**Windows Example:**
```batch
build-windows.bat static
# Result: build\Release\VSTRunner.exe
# Just copy this ONE file anywhere and run it!
```

**Linux Example:**
```bash
./build-linux.sh static
# Result: build/VSTRunner
# Just copy this ONE file and run: ./VSTRunner
```

**Note:** Even "static" builds may require some system libraries:
- Windows: Requires Windows Vista+ (no additional DLLs)
- Linux: May require basic X11 libraries (but not JUCE/STL)
- macOS: .app bundles are already self-contained

### 3. Installer Package

**What it does:**
- Creates professional installer package
- Handles installation, shortcuts, uninstallation
- Registers file associations (optional)
- Adds to Start Menu/Applications folder

**When to use:**
- Professional distribution
- Enterprise deployment
- Users want proper installation
- Need uninstaller

**Output:**
- Windows: `VSTRunner-1.0.0-win64.exe` (NSIS installer)
- macOS: `VSTRunner-1.0.0-Darwin.dmg` (DMG disk image)
- Linux: `.deb`, `.rpm`, and `.tar.gz` packages

---

## Detailed Build Instructions

### Prerequisites

#### All Platforms
- **CMake 3.15+** ([Download](https://cmake.org/download/))
- **Git** (for JUCE submodule)
- **JUCE submodule initialized** (automatic in build scripts)

#### Windows
- **Visual Studio 2019 or 2022** with:
  - C++ desktop development workload
  - Windows 10/11 SDK
- Optional: **NSIS** for installer generation

#### macOS
- **Xcode 12+** with Command Line Tools:
  ```bash
  xcode-select --install
  ```

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get install -y \
    build-essential cmake git pkg-config \
    libasound2-dev libfreetype6-dev \
    libx11-dev libxext-dev libxrandr-dev \
    libxinerama-dev libxcursor-dev \
    libjack-jackd2-dev
```

#### Linux (Fedora/RHEL)
```bash
sudo dnf install -y \
    gcc-c++ cmake git pkgconfig \
    alsa-lib-devel freetype-devel \
    libX11-devel libXext-devel libXrandr-devel \
    libXinerama-devel libXcursor-devel \
    jack-audio-connection-kit-devel
```

---

## Manual Build (Advanced)

If you prefer to build manually without scripts:

### 1. Initialize JUCE submodule
```bash
git submodule update --init --recursive
```

### 2. Configure CMake

**Standard build:**
```bash
mkdir build && cd build
cmake ..
```

**Portable static build:**
```bash
mkdir build && cd build
cmake -DBUILD_STATIC=ON ..
```

**With installer:**
```bash
mkdir build && cd build
cmake -DBUILD_STATIC=ON -DBUILD_INSTALLER=ON ..
```

### 3. Build

**Windows (Visual Studio):**
```bash
cmake --build . --config Release
```

**macOS/Linux:**
```bash
cmake --build . -- -j$(nproc)
```

### 4. Generate installer (if BUILD_INSTALLER=ON)
```bash
cpack -C Release
```

---

## CMake Build Options

You can customize the build with these options:

| Option | Values | Description |
|--------|--------|-------------|
| `CMAKE_BUILD_TYPE` | `Release`, `Debug` | Build configuration (Release for distribution) |
| `BUILD_STATIC` | `ON`, `OFF` | Enable static linking for portable executable |
| `BUILD_INSTALLER` | `ON`, `OFF` | Generate installer package |

**Example:**
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON -DBUILD_INSTALLER=ON ..
```

---

## Distribution Recommendations

### For Most Users (Like GHelper)

**Windows:**
```batch
build-windows.bat static
```
→ Distribute: `build\Release\VSTRunner.exe` (single file)

**Linux:**
```bash
./build-linux.sh static
```
→ Distribute: `build/VSTRunner` (single file)

**macOS:**
```bash
./build-macos.sh
```
→ Distribute: `VSTRunner.app` (already self-contained)

### For Professional/Enterprise

**All Platforms:**
Use installer packages:
- Windows: `build-windows.bat installer` → `.exe` installer
- macOS: `./build-macos.sh installer` → `.dmg` disk image
- Linux: `./build-linux.sh installer` → `.deb`, `.rpm`, `.tar.gz`

### For Package Managers

**Regular builds** without static linking:
- Windows: Use regular build, distribute via Chocolatey/winget
- macOS: Use regular build, distribute via Homebrew
- Linux: Use regular build, distribute via apt/dnf/pacman

---

## File Sizes (Approximate)

| Platform | Regular | Static/Portable | Installer |
|----------|---------|-----------------|-----------|
| Windows x64 | 3-5 MB | 12-18 MB | 15-25 MB |
| macOS (Universal) | 8-12 MB | N/A (bundled) | 10-15 MB |
| Linux x64 | 4-6 MB | 18-30 MB | 20-35 MB |

**Note:** Actual sizes depend on:
- Compiler optimizations
- Included JUCE modules
- Debug symbols (Release builds have them stripped)

---

## Testing Your Build

After building, verify functionality:

### 1. Application Launches
```bash
# Windows
build\Release\VSTRunner.exe

# macOS
open build/Release/VSTRunner.app

# Linux
./build/VSTRunner
```

### 2. Check Dependencies (Linux/macOS)

**Linux:**
```bash
ldd build/VSTRunner
```
- Static build should show fewer dependencies
- Regular build shows all system libraries

**macOS:**
```bash
otool -L build/Release/VSTRunner.app/Contents/MacOS/VSTRunner
```

### 3. Test Core Features
- [ ] Application opens without errors
- [ ] Audio settings accessible
- [ ] Can browse for plugins
- [ ] Can load a VST plugin
- [ ] Audio processing works
- [ ] Web server starts/stops
- [ ] Settings persist between sessions

---

## Troubleshooting

### Build Errors

**"JUCE not found"**
```bash
git submodule update --init --recursive
```

**"Missing dependencies" (Linux)**
```bash
# See Prerequisites section above
sudo apt-get install <missing-package>
```

**Visual Studio build fails (Windows)**
- Ensure you have C++ desktop workload installed
- Use Visual Studio 2019 or 2022
- Try running build script from "Developer Command Prompt"

### Runtime Errors

**Missing DLLs (Windows)**
- Use static build: `build-windows.bat static`
- Or install Visual C++ Redistributable

**Library errors (Linux)**
- Check dependencies: `ldd ./build/VSTRunner`
- Install missing libraries via package manager
- Or use static build: `./build-linux.sh static`

**Code signing errors (macOS)**
- Self-signed apps may require: System Preferences → Security → Allow
- For distribution, sign with Apple Developer certificate

---

## Code Signing & Notarization

### macOS

For public distribution:

```bash
# Sign the app
codesign --deep --force --verify --verbose \
    --sign "Developer ID Application: Your Name" \
    build/Release/VSTRunner.app

# Notarize (requires Apple Developer account)
xcrun notarytool submit VSTRunner.dmg \
    --apple-id "your@email.com" \
    --password "app-specific-password" \
    --team-id "TEAM_ID"
```

### Windows

Optional (for avoiding SmartScreen warnings):

```bash
# Sign with code signing certificate
signtool sign /f certificate.pfx /p password \
    /t http://timestamp.digicert.com \
    build\Release\VSTRunner.exe
```

---

## Automated Builds (CI/CD)

See `.github/workflows/` (if available) for automated build configurations.

**Example GitHub Actions workflow:**
- Builds for Windows, macOS, Linux
- Generates installers
- Creates release artifacts
- Uploads to releases page

---

## Questions?

- **Build issues?** Check BUILD.md for detailed build instructions
- **Usage questions?** See SETUP.md
- **Development?** See GETTING_STARTED.md

---

## Summary

**For "GHelper-style" single-file distribution:**

| Platform | Command | Output File |
|----------|---------|-------------|
| Windows | `build-windows.bat static` | `build\Release\VSTRunner.exe` |
| Linux | `./build-linux.sh static` | `build/VSTRunner` |
| macOS | `./build-macos.sh` | `build/Release/VSTRunner.app` |

**Just copy the output file and distribute!** Users can run it directly without installation.

**For installer packages:**

| Platform | Command | Output File |
|----------|---------|-------------|
| Windows | `build-windows.bat installer` | `VSTRunner-1.0.0-win64.exe` |
| Linux | `./build-linux.sh installer` | `.deb`, `.rpm`, `.tar.gz` |
| macOS | `./build-macos.sh installer` | `VSTRunner-1.0.0-Darwin.dmg` |

Choose based on your distribution needs!
