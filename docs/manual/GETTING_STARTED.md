# Getting Started with VST Runner

A comprehensive guide for developers new to C++ and JUCE.

## Table of Contents

1. [What is VST Runner?](#what-is-vst-runner)
2. [Tech Stack Overview](#tech-stack-overview)
3. [Architecture](#architecture)
4. [Project Structure](#project-structure)
5. [Build System](#build-system)
6. [Key Concepts](#key-concepts)
7. [Code Walkthrough](#code-walkthrough)
8. [How to Extend](#how-to-extend)

---

## What is VST Runner?

VST Runner is a **desktop application** that lets you run VST audio plugins as standalone programs. Think of it as a lightweight host for testing and using audio plugins without needing a full Digital Audio Workstation (DAW).

**Key Features:**
- Load VST2/VST3 plugins
- Process audio in real-time
- Control via desktop UI or web browser
- Settings persistence
- ASIO/CoreAudio support

**Use Cases:**
- Test new plugins you've downloaded
- Use a single effect/instrument live
- Remote control plugins from tablet/phone
- Quick sound design without opening a DAW

---

## Tech Stack Overview

### Core Technologies

#### 1. **C++ (Language)**

**What it is:** A powerful, compiled programming language.

**Why we use it:**
- **Performance**: Real-time audio processing needs speed
- **Industry standard**: Most audio software is written in C++
- **Native**: Direct access to hardware (audio interfaces, MIDI)
- **VST SDK**: Plugin SDKs are written in C++

**Learning resources:**
- [LearnCpp.com](https://www.learncpp.com/)
- [C++ Reference](https://en.cppreference.com/)

#### 2. **JUCE (Framework)**

**What it is:** A cross-platform C++ framework for audio applications.

**Why we use it:**
- **Cross-platform**: Write once, run on Windows, macOS, Linux
- **Audio support**: Built-in audio device management (ASIO, CoreAudio, ALSA)
- **Plugin hosting**: Can load and run VST/VST3/AU plugins
- **GUI**: Built-in UI components and windowing
- **Industry standard**: Used by many commercial audio apps

**JUCE provides:**
```
juce::AudioDeviceManager    → Manages audio hardware
juce::AudioPluginInstance   → Represents a loaded plugin
juce::AudioProcessor        → Base class for audio processing
juce::Component             → Base class for UI elements
juce::StreamingSocket       → Network communication
```

**Learning resources:**
- [JUCE Documentation](https://docs.juce.com/)
- [JUCE Tutorials](https://juce.com/learn/tutorials)
- [TheAudioProgrammer (YouTube)](https://www.youtube.com/c/TheAudioProgrammer)

#### 3. **CMake (Build System)**

**What it is:** A tool that generates build files for your project.

**Why we use it:**
- **Cross-platform**: Same CMakeLists.txt works on Windows, macOS, Linux
- **JUCE integration**: JUCE provides CMake functions
- **IDE support**: Generates projects for Visual Studio, Xcode, etc.

**What it does:**
1. Reads `CMakeLists.txt`
2. Finds dependencies (JUCE)
3. Generates build files (Makefiles, .sln, .xcodeproj)
4. Compiles your code

#### 4. **VST SDK (Plugin Format)**

**What it is:** Software Development Kit for VST (Virtual Studio Technology) plugins.

**Plugin formats supported:**
- **VST2**: Older format (.dll on Windows, .vst on macOS)
- **VST3**: Modern format (.vst3 folder/bundle)
- **AU**: Audio Units (macOS only)
- **AAX**: Pro Tools format (future support)

**What is a plugin?**
A plugin is a small program that processes audio:
- **Effects**: Reverb, delay, EQ, compressor
- **Instruments**: Synthesizers, samplers, drum machines

#### 5. **Web Technologies (HTML/CSS/JavaScript)**

**Used for:** Web interface (remote control)

**Why:**
- Access from any device with a browser
- No native app needed for remote control
- Easy to design and iterate on UI

---

## Architecture

VST Runner uses a **layered architecture** with clear separation of concerns.

### High-Level Architecture

```
┌─────────────────────────────────────────────┐
│                  UI Layer                   │
│  ┌──────────────┐      ┌─────────────────┐ │
│  │  Desktop UI  │      │  Web Interface  │ │
│  │ (MainComponent)     │  (HTML/JS)      │ │
│  └──────┬───────┘      └────────┬────────┘ │
└─────────┼──────────────────────┼───────────┘
          │                      │
          │    ┌─────────────────┘
          │    │
┌─────────▼────▼───────────────────────────────┐
│              Control Layer                   │
│  ┌────────────┐ ┌──────────────┐ ┌────────┐ │
│  │   Plugin   │ │   AudioIO    │ │ Config │ │
│  │  Manager   │ │   Manager    │ │Manager │ │
│  └─────┬──────┘ └──────┬───────┘ └────┬───┘ │
└────────┼────────────────┼──────────────┼─────┘
         │                │              │
         │    ┌───────────┴──────────────┘
         │    │
┌────────▼────▼──────────────────────────────┐
│          Audio Processing Layer            │
│     ┌─────────────────────────────┐        │
│     │  PluginHostProcessor        │        │
│     │  - Audio callback           │        │
│     │  - Real-time processing     │        │
│     └──────────┬──────────────────┘        │
└────────────────┼────────────────────────────┘
                 │
┌────────────────▼────────────────────────────┐
│           Hardware Layer                    │
│     ┌────────────────────────────┐          │
│     │  AudioDeviceManager        │          │
│     │  - ASIO/CoreAudio/ALSA     │          │
│     │  - Audio interface         │          │
│     └────────────────────────────┘          │
└─────────────────────────────────────────────┘
```

### Component Responsibilities

#### **UI Layer** (User Interaction)
- **MainComponent**: Desktop window with controls
- **WebServer**: HTTP server for web interface
- **Handles**: User input, display updates

#### **Control Layer** (Business Logic)
- **PluginManager**: Loading/unloading plugins
- **AudioIOManager**: Gain control, routing
- **ConfigurationManager**: Settings persistence
- **Handles**: Application logic, state management

#### **Audio Processing Layer** (Real-Time)
- **PluginHostProcessor**: Audio pipeline
- **Handles**: Audio callback, plugin processing

#### **Hardware Layer** (System)
- **AudioDeviceManager**: Hardware abstraction
- **Handles**: Audio driver communication

### Design Patterns Used

#### 1. **Manager Pattern**
**Where**: PluginManager, AudioIOManager, ConfigurationManager

**What it is**: Centralized control for a specific domain.

**Why**:
- Single source of truth
- Easy to test
- Clear responsibilities

#### 2. **Singleton Pattern**
**Where**: ConfigurationManager

**What it is**: Only one instance exists globally.

**Why**: Settings should be global and consistent.

#### 3. **Observer Pattern (Listener)**
**Where**: PluginManager::Listener, AudioIOManager::Listener

**What it is**: Objects notify others when state changes.

**Why**: Loose coupling, UI can react to changes.

#### 4. **Composition**
**Where**: PluginHostProcessor contains managers

**What it is**: Building complex objects from simpler ones.

**Why**: More flexible than inheritance.

---

## Project Structure

```
vst-runner/
│
├── JUCE/                      # JUCE framework (git submodule)
│   └── modules/               # JUCE's audio/GUI modules
│
├── Source/                    # Your application code
│   │
│   ├── Core/                  # Business logic (no UI)
│   │   ├── ConfigurationManager.{h,cpp}
│   │   ├── PluginManager.{h,cpp}
│   │   └── AudioIOManager.{h,cpp}
│   │
│   ├── Web/                   # Web interface
│   │   └── Resources/
│   │       ├── index.html     # Web UI
│   │       └── app.js         # Client JavaScript
│   │
│   ├── Main.cpp               # Application entry point
│   ├── MainComponent.{h,cpp}  # Main window UI
│   ├── AudioSettingsComponent.{h,cpp}
│   ├── PluginHostProcessor.{h,cpp}
│   └── WebServer.{h,cpp}
│
├── build/                     # Generated build files (gitignored)
│   └── VSTRunner_artefacts/   # Compiled executables
│
├── CMakeLists.txt             # Build configuration
├── README.md                  # Project overview
├── BUILD.md                   # Build instructions
├── SETUP.md                   # Usage guide
└── GETTING_STARTED.md         # This file
```

### Key Files Explained

#### **CMakeLists.txt**
Build configuration. Tells CMake:
- Project name and version
- Source files to compile
- Libraries to link (JUCE)
- Compiler settings

#### **Main.cpp**
Entry point. Creates the application window.

```cpp
// Simplified version
int main() {
    auto app = new VSTRunnerApplication();
    app->initialise();  // Creates main window
    app->run();         // Event loop
    return 0;
}
```

#### **MainComponent.cpp**
Main UI window. Contains:
- UI elements (buttons, sliders, labels)
- Event handlers (button clicks)
- Timer for UI updates

#### **PluginHostProcessor.cpp**
Audio processor. Contains:
- `prepareToPlay()`: Called when audio starts
- `processBlock()`: Called for each audio buffer
- `releaseResources()`: Called when audio stops

#### **Core Managers**
Business logic separated from UI:
- **ConfigurationManager**: Load/save settings
- **PluginManager**: Load/scan plugins
- **AudioIOManager**: Gain and routing

---

## Build System

### How CMake Works

```
1. Configure Phase
   cmake ..
   └─> Reads CMakeLists.txt
   └─> Finds JUCE
   └─> Generates build files

2. Build Phase
   cmake --build .
   └─> Compiles C++ code
   └─> Links libraries
   └─> Creates executable

3. Run Phase
   ./VSTRunner_artefacts/Debug/VSTRunner
   └─> Runs your application
```

### CMakeLists.txt Breakdown

```cmake
# Set project details
project(VSTRunner VERSION 1.0.0)

# Add JUCE framework
add_subdirectory(JUCE)

# Create executable
juce_add_gui_app(VSTRunner ...)

# List source files
target_sources(VSTRunner PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
    ...
)

# Link JUCE modules
target_link_libraries(VSTRunner PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    ...
)

# Set compiler flags
target_compile_definitions(VSTRunner PRIVATE
    JUCE_PLUGINHOST_VST3=1
    ...
)
```

### Build Configurations

**Debug**:
- Slower execution
- Includes debugging symbols
- Easier to debug with GDB/LLDB
- Use during development

**Release**:
- Optimized code
- Faster execution
- No debug symbols
- Use for distribution

```bash
# Debug build (default)
cmake --build .

# Release build
cmake --build . --config Release
```

---

## Key Concepts

### 1. Audio Processing

#### Audio Buffer
```cpp
juce::AudioBuffer<float> buffer;
// Contains audio samples
// buffer[channel][sample]
```

**Example:**
- **Channels**: 2 (stereo: left, right)
- **Buffer size**: 512 samples
- **Sample rate**: 48000 Hz
- **Duration**: 512 / 48000 = 10.67 milliseconds

#### Audio Callback

```cpp
void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midi) {
    // Called every ~10ms
    // Process buffer in real-time
    // MUST be fast (no allocations, no locks)
}
```

**Rules for audio thread:**
- ❌ No memory allocations (malloc, new)
- ❌ No file I/O
- ❌ No locks/mutexes (use atomics)
- ❌ No long operations
- ✅ Only process the buffer

### 2. Plugin Hosting

#### Plugin Lifecycle

```
1. Scan         → Find plugin file
2. Describe     → Read plugin metadata
3. Instantiate  → Create plugin object
4. Prepare      → Set sample rate, buffer size
5. Process      → Call processBlock()
6. Release      → Clean up
7. Destroy      → Delete plugin object
```

#### Plugin Processing

```cpp
// In PluginHostProcessor::processBlock()
void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midi) {
    // 1. Apply input gain
    audioIOManager.processInputGain(buffer);

    // 2. Filter MIDI
    audioIOManager.processMidiInput(midi);

    // 3. Process plugin
    plugin->processBlock(buffer, midi);

    // 4. Apply output gain
    audioIOManager.processOutputGain(buffer);
}
```

### 3. UI Components

#### JUCE Component Hierarchy

```
Component (base class)
├── Button
├── Slider
├── Label
├── TextEditor
└── Your custom components
    └── MainComponent
```

#### Event Handling

```cpp
// Create button
juce::TextButton myButton;

// Set callback
myButton.onClick = [this] {
    // Called when button clicked
    doSomething();
};
```

#### Painting

```cpp
void paint(Graphics& g) override {
    // Called when component needs redrawing
    g.fillAll(Colours::black);
    g.setColour(Colours::white);
    g.drawText("Hello", bounds, Justification::centred);
}
```

### 4. Thread Safety

#### Threads in VST Runner

**1. UI Thread** (Main Thread)
- Handles user input
- Updates display
- CAN allocate memory
- CAN use locks

**2. Audio Thread** (Real-Time Thread)
- Processes audio
- CANNOT allocate memory
- CANNOT use locks
- MUST be fast

**3. Web Server Thread**
- Handles HTTP requests
- CAN allocate memory
- CAN use locks

#### Communication Between Threads

**Atomics** (lock-free):
```cpp
std::atomic<float> gain { 1.0f };

// UI thread
gain.store(1.5f);

// Audio thread
float g = gain.load();
buffer.applyGain(g);
```

**Message Queue** (JUCE):
```cpp
// Audio thread → UI thread
MessageManager::callAsync([this] {
    // Runs on UI thread
    updateDisplay();
});
```

### 5. Settings Persistence

```cpp
// Save
ConfigurationManager::getInstance().setWindowBounds(bounds);
// Automatically saved to:
// ~/.config/VSTRunner/settings.xml (Linux)
// ~/Library/Application Support/VSTRunner/settings.xml (macOS)
// C:\Users\You\AppData\Roaming\VSTRunner\settings.xml (Windows)

// Load
auto bounds = ConfigurationManager::getInstance().getWindowBounds();
```

---

## Code Walkthrough

Let's trace what happens when you load a plugin:

### Step-by-Step: Loading a Plugin

#### 1. User clicks "Load Plugin" button

```cpp
// In MainComponent.cpp
loadButton.onClick = [this] {
    loadPluginFromPath(pluginPathEditor.getText());
};
```

#### 2. MainComponent calls processor

```cpp
void MainComponent::loadPluginFromPath(const String& path) {
    File pluginFile(path);

    // Close any existing plugin editor
    closePluginEditor();

    // Load the plugin via processor
    if (pluginProcessor->loadPlugin(pluginFile)) {
        // Success!
        showPluginEditor();
    }
}
```

#### 3. PluginHostProcessor delegates to PluginManager

```cpp
// In PluginHostProcessor.cpp
bool PluginHostProcessor::loadPlugin(const File& pluginPath) {
    String errorMessage;

    // Delegate to PluginManager
    auto* plugin = pluginManager.loadPlugin(pluginPath, errorMessage);

    if (plugin == nullptr) {
        // Show error
        return false;
    }

    // Prepare plugin
    if (getSampleRate() > 0) {
        plugin->prepareToPlay(getSampleRate(), getBlockSize());
    }

    return true;
}
```

#### 4. PluginManager does the heavy lifting

```cpp
// In PluginManager.cpp
AudioPluginInstance* PluginManager::loadPlugin(
    const File& pluginPath,
    String& errorMessage)
{
    // 1. Unload existing plugin
    unloadPlugin();

    // 2. Scan plugin file
    OwnedArray<PluginDescription> typesFound;
    for (auto* format : formatManager.getFormats()) {
        format->findAllTypesForFile(typesFound, pluginPath);
        if (typesFound.size() > 0) break;
    }

    if (typesFound.isEmpty()) {
        errorMessage = "Invalid plugin file";
        return nullptr;
    }

    // 3. Instantiate plugin
    loadedPlugin.reset(
        formatManager.createPluginInstance(
            *typesFound[0],
            44100.0,  // Sample rate
            512,      // Buffer size
            errorMessage
        ).release()
    );

    // 4. Notify listeners
    if (loadedPlugin != nullptr) {
        notifyPluginLoaded(loadedPlugin->getName());
    }

    return loadedPlugin.get();
}
```

#### 5. Plugin is now loaded and ready

```cpp
// In audio callback
void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midi) {
    if (auto* plugin = pluginManager.getLoadedPlugin()) {
        // Process audio through plugin
        plugin->processBlock(buffer, midi);
    }
}
```

### Data Flow Diagram

```
User clicks
    │
    ▼
MainComponent::loadPluginFromPath()
    │
    ▼
PluginHostProcessor::loadPlugin()
    │
    ▼
PluginManager::loadPlugin()
    │
    ├─> Scan plugin file
    ├─> Create plugin instance
    └─> Notify listeners
    │
    ▼
Plugin ready
    │
    ▼
Audio thread calls processBlock()
    │
    ▼
Plugin processes audio
```

---

## How to Extend

### Adding a New Feature

Let's say you want to add **plugin preset management**.

#### 1. Choose the right manager

Presets are plugin-related → **PluginManager**

#### 2. Add methods to PluginManager.h

```cpp
class PluginManager {
public:
    // ... existing methods ...

    // NEW: Preset management
    void savePreset(const String& presetName);
    void loadPreset(const String& presetName);
    Array<String> getPresetList() const;

private:
    File getPresetDirectory() const;
    // ... existing members ...
};
```

#### 3. Implement in PluginManager.cpp

```cpp
void PluginManager::savePreset(const String& presetName) {
    if (!hasPluginLoaded()) return;

    // Get plugin state
    MemoryBlock state;
    loadedPlugin->getStateInformation(state);

    // Save to file
    File presetFile = getPresetDirectory()
        .getChildFile(presetName + ".preset");
    presetFile.replaceWithData(state.getData(), state.getSize());
}

File PluginManager::getPresetDirectory() const {
    return File::getSpecialLocation(File::userDocumentsDirectory)
        .getChildFile("VSTRunner")
        .getChildFile("Presets");
}
```

#### 4. Add UI controls in MainComponent

```cpp
// In MainComponent.h
class MainComponent : public Component {
private:
    TextButton savePresetButton;
    ComboBox presetSelector;
};

// In MainComponent.cpp constructor
savePresetButton.setButtonText("Save Preset");
savePresetButton.onClick = [this] {
    auto& pm = pluginProcessor->getPluginManager();
    pm.savePreset("MyPreset");
};
addAndMakeVisible(savePresetButton);
```

#### 5. Test your feature

```bash
# Rebuild
cd build
cmake --build .

# Run and test
./VSTRunner_artefacts/Debug/VSTRunner
```

### Common Extension Points

#### 1. Add new settings to ConfigurationManager
```cpp
// Add to ConfigurationManager.h
bool getShowAdvancedOptions() const;
void setShowAdvancedOptions(bool show);

// Update XML save/load methods
```

#### 2. Add audio metering to AudioIOManager
```cpp
// Add to AudioIOManager.h
float getCurrentInputLevel() const;
float getCurrentOutputLevel() const;

// In processBlock(), calculate RMS
```

#### 3. Add plugin scanning to PluginManager
```cpp
// Already has scanDirectory() method!
// Just need to add UI for it
```

#### 4. Add new web API endpoints
```cpp
// In WebServer.cpp
if (path.startsWith("/api/presets/list")) {
    response = handleGetPresets();
}
```

---

## Learning Resources

### C++ Basics
- [LearnCpp.com](https://www.learncpp.com/) - Best beginner tutorial
- [C++ Primer](https://www.amazon.com/Primer-5th-Stanley-B-Lippman/dp/0321714113) - Book
- [CppReference](https://en.cppreference.com/) - Reference docs

### JUCE Framework
- [JUCE Tutorials](https://juce.com/learn/tutorials) - Official tutorials
- [JUCE Documentation](https://docs.juce.com/) - API reference
- [TheAudioProgrammer](https://www.youtube.com/c/TheAudioProgrammer) - Video tutorials
- [JUCE Forum](https://forum.juce.com/) - Community help

### Audio Programming
- [The Audio Programmer](https://www.youtube.com/c/TheAudioProgrammer) - YouTube channel
- [Designing Audio Effect Plugins in C++](https://www.amazon.com/Designing-Audio-Effect-Plugins-C/dp/1138591939) - Book
- [MusicDSP.org](https://www.musicdsp.org/) - DSP algorithms

### VST Development
- [Steinberg VST3 SDK](https://github.com/steinbergmedia/vst3sdk) - Official SDK
- [VST3 Documentation](https://steinbergmedia.github.io/vst3_doc/) - Spec docs

### CMake
- [CMake Tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/index.html) - Official
- [Modern CMake](https://cliutils.gitlab.io/modern-cmake/) - Best practices

---

## Next Steps

### 1. Build and run the application
Follow [BUILD.md](BUILD.md) to compile and run.

### 2. Explore the code
- Start with `Main.cpp` (entry point)
- Read `MainComponent.cpp` (UI)
- Look at `PluginHostProcessor.cpp` (audio)
- Check out the Core managers

### 3. Make small changes
- Change window size in `MainComponent.cpp`
- Add a button to the UI
- Add a new setting to ConfigurationManager

### 4. Read JUCE tutorials
Work through the official JUCE tutorials to understand:
- Audio basics
- GUI components
- Plugin hosting

### 5. Ask questions!
- JUCE Forum: https://forum.juce.com/
- Audio Programming Discord servers
- Stack Overflow (tag: juce)

---

## Glossary

**ASIO**: Audio Stream Input/Output - Low-latency audio driver (Windows)

**Audio Callback**: Function called by the OS to process audio in real-time

**Buffer**: Array of audio samples processed together

**Component**: JUCE's base class for UI elements

**CoreAudio**: Apple's audio framework (macOS/iOS)

**DAW**: Digital Audio Workstation (e.g., Ableton, Logic, FL Studio)

**DSP**: Digital Signal Processing - Audio algorithms

**JUCE**: Jules' Utility Class Extensions - C++ framework

**Latency**: Delay between input and output

**Plugin**: Small program that processes audio

**Real-time**: Processing that must complete within a time constraint

**Sample Rate**: Number of audio samples per second (e.g., 48000 Hz)

**VST**: Virtual Studio Technology - Plugin format by Steinberg

**VST3**: Modern version of VST

---

## Troubleshooting

### Build Issues

**"JUCE not found"**
```bash
git submodule update --init --recursive
```

**"Missing dependencies"**
Install system packages (see BUILD.md)

**"Compiler error"**
Check C++ standard is set to C++17

### Runtime Issues

**"No audio devices"**
Check audio settings, make sure device isn't used by another app

**"Plugin won't load"**
- Check plugin is 64-bit (not 32-bit)
- Check plugin file path is correct
- Try the plugin in another host (DAW)

**"Crashes on startup"**
Run in debugger to see stack trace

### Getting Help

1. Check existing documentation
2. Search JUCE forum
3. Ask in audio programming communities
4. Open GitHub issue with:
   - OS and version
   - Error message
   - Steps to reproduce

---

## Summary

VST Runner is a **cross-platform audio application** built with:
- **C++** for performance
- **JUCE** for audio and GUI
- **CMake** for building
- **Manager pattern** for organization

Key concepts:
- Audio processing is real-time
- Plugins are separate programs loaded dynamically
- UI and audio run on different threads
- Settings are persisted between sessions

Start with the tutorials, explore the code, and experiment!

**Welcome to audio programming!** 🎵
