# Building VST Runner on Windows

You're currently in **WSL** (Windows Subsystem for Linux), but for best audio support, you should build **natively on Windows**.

## ⚠️ WSL Limitations

WSL does **not** have:
- Direct access to Windows audio devices
- ASIO driver support
- Native MIDI support
- Full GUI audio capabilities

**Result:** VST Runner will build in WSL but won't have working audio.

---

## ✅ RECOMMENDED: Build on Native Windows

### Prerequisites

1. **Visual Studio 2019 or 2022** (Community Edition is free)
   - Download: https://visualstudio.microsoft.com/downloads/
   - Install the "Desktop development with C++" workload
   - Install the Windows 10/11 SDK

2. **Git for Windows** (if not already installed)
   - Download: https://git-scm.com/download/win

### Build Steps

**Option 1: Using the Build Script (Easiest)**

1. Open Windows Terminal or PowerShell (not WSL)
2. Navigate to project:
   ```powershell
   cd C:\Users\satmo\projects\vst-runner
   ```

3. Initialize JUCE submodule (first time only):
   ```powershell
   git submodule update --init --recursive
   ```

4. Build portable executable:
   ```batch
   build-windows.bat static
   ```

5. Your executable will be at:
   ```
   C:\Users\satmo\projects\vst-runner\build\Release\VSTRunner.exe
   ```

**Option 2: Manual Build with CMake**

1. Open "x64 Native Tools Command Prompt for VS 2022" from Start Menu

2. Navigate to project:
   ```batch
   cd C:\Users\satmo\projects\vst-runner
   ```

3. Create build directory:
   ```batch
   mkdir build
   cd build
   ```

4. Configure:
   ```batch
   cmake -G "Visual Studio 17 2022" -A x64 -DBUILD_STATIC=ON ..
   ```

5. Build:
   ```batch
   cmake --build . --config Release
   ```

6. Executable location:
   ```
   build\Release\VSTRunner.exe
   ```

---

## 🐧 If You Still Want to Build in WSL

Despite audio limitations, if you want to build in WSL (for testing UI only):

### 1. Navigate to project directory
```bash
cd /mnt/c/Users/satmo/projects/vst-runner
```

### 2. Install dependencies
```bash
sudo apt-get install -y \
    build-essential cmake git pkg-config \
    libasound2-dev libjack-jackd2-dev \
    libfreetype-dev libx11-dev libxcomposite-dev \
    libxcursor-dev libxext-dev libxinerama-dev \
    libxrandr-dev libxrender-dev libglu1-mesa-dev \
    mesa-common-dev
```

### 3. Initialize JUCE (if not done)
```bash
git submodule update --init --recursive
```

### 4. Build
```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)
```

### 5. Try to run (GUI may not work properly)
```bash
cd ..
./build/VSTRunner
```

**Note:** You'll see "no soundcards found" because WSL doesn't have audio device access.

---

## 🎯 Quick Decision Guide

**Want working audio and VST plugins?**
→ Build on **native Windows** using Visual Studio

**Just want to test compilation?**
→ Can build in WSL, but audio won't work

**Want to distribute to users?**
→ Use **native Windows** with `build-windows.bat static`

---

## 🔧 Troubleshooting

### Visual Studio Not Found
Make sure you have:
- Visual Studio 2019 or 2022 installed
- "Desktop development with C++" workload
- Windows SDK

### CMake Not Found
Install CMake:
```powershell
winget install Kitware.CMake
```
Or download from: https://cmake.org/download/

### JUCE Submodule Missing
```bash
git submodule update --init --recursive
```

### Build Fails on Windows
- Make sure you're using "x64 Native Tools Command Prompt"
- Not regular Command Prompt or PowerShell
- Find it in: Start Menu → Visual Studio 2022 → x64 Native Tools Command Prompt

---

## 📦 After Building

Once you have `VSTRunner.exe`:

1. **Test it:**
   - Double-click VSTRunner.exe
   - Click "Audio Settings" to configure ASIO/WASAPI
   - Load a VST plugin

2. **Distribute it:**
   - The static build is portable (12-18 MB)
   - Copy just the .exe file
   - Users can run it without installation

3. **Create installer (optional):**
   ```batch
   build-windows.bat installer
   ```
   Creates a proper Windows installer (.exe)

---

## 🆘 Need Help?

**Current location:** WSL (`/mnt/c/Users/satmo/projects/vst-runner`)
**Windows path:** `C:\Users\satmo\projects\vst-runner`

To access this in Windows:
1. Open Windows Explorer
2. Navigate to: `C:\Users\satmo\projects\vst-runner`
3. Run `build-windows.bat` from there
