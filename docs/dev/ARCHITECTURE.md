# VST Runner Architecture

This document describes the architecture of VST Runner, focusing on the Phase 3 refactored design that emphasizes modularity, testability, and maintainability.

## Table of Contents

- [Overview](#overview)
- [Architecture Principles](#architecture-principles)
- [System Components](#system-components)
- [Design Patterns](#design-patterns)
- [Component Details](#component-details)
- [Data Flow](#data-flow)
- [Testing Strategy](#testing-strategy)

## Overview

VST Runner is a cross-platform VST plugin host with both desktop and web interfaces. The architecture follows a layered approach with clear separation between:

- **Core Layer**: Business logic and plugin management
- **UI Layer**: User interface components and panels
- **Web Layer**: HTTP server and REST API
- **Audio Layer**: JUCE audio processing and device management

```
┌─────────────────────────────────────────────────────────────┐
│                        Application                          │
│                         (Main.cpp)                          │
└────────────────────┬────────────────────────────────────────┘
                     │
        ┌────────────┴────────────┐
        │                         │
┌───────▼────────┐       ┌───────▼────────┐
│   UI Layer     │       │   Web Layer    │
│  MainComponent │       │   WebServer    │
└───────┬────────┘       └───────┬────────┘
        │                        │
        │                ┌───────▼────────┐
        │                │ WebAPIHandler  │
        │                └───────┬────────┘
        │                        │
        └────────────┬───────────┘
                     │
            ┌────────▼─────────┐
            │   Core Layer     │
            │   - Managers     │
            │   - Processor    │
            └────────┬─────────┘
                     │
            ┌────────▼─────────┐
            │   Audio Layer    │
            │  JUCE Framework  │
            └──────────────────┘
```

## Architecture Principles

### 1. Separation of Concerns

Each component has a single, well-defined responsibility:

- **PluginHostProcessor**: Audio processing and plugin hosting
- **ConfigurationManager**: Configuration state management
- **PluginManager**: Plugin lifecycle and operations
- **AudioIOManager**: Audio/MIDI I/O settings
- **WebServer**: HTTP protocol handling
- **WebAPIHandler**: REST API business logic
- **UI Panels**: Modular presentation logic

### 2. Dependency Inversion

High-level modules (UI, Web) depend on abstractions (Manager interfaces) rather than concrete implementations. This allows for:

- Easy unit testing with mocks
- Component replacement without cascading changes
- Clear contracts between layers

### 3. Composition Over Inheritance

- **UI Panels**: MainComponent composes three independent panels
- **Managers**: PluginHostProcessor composes three manager classes
- Callbacks enable loose coupling without inheritance hierarchies

### 4. Single Responsibility Principle

Each class has one reason to change:

- **WebServer**: HTTP protocol changes
- **WebAPIHandler**: API endpoint logic changes
- **PluginManager**: Plugin loading logic changes

## System Components

### Core Layer (Source/Core/)

**PluginHostProcessor.h/cpp** (420 lines)
- Main audio processor
- Composes ConfigurationManager, PluginManager, AudioIOManager
- Coordinates audio processing pipeline
- Implements JUCE AudioProcessor interface

**ConfigurationManager.h/cpp** (145 lines)
- Manages application configuration state
- Notifies observers of configuration changes
- Thread-safe configuration access

**PluginManager.h/cpp** (180 lines)
- Plugin loading and unloading
- Plugin validation and metadata
- Plugin scanner integration
- Observer pattern for plugin state changes

**AudioIOManager.h/cpp** (135 lines)
- Audio input/output gain control
- Audio/MIDI input enable/disable
- Real-time parameter changes
- Thread-safe parameter access

### UI Layer (Source/UI/)

**MainComponent.h/cpp** (68 + 322 lines)
- Main application window
- Composes three UI panels
- Orchestrates business logic callbacks
- Timer-based UI updates

**PluginControlPanel.h/cpp** (70 + 140 lines)
- Plugin path input and browsing
- Plugin load button and status
- Show editor button
- Callback-based communication

**AudioControlPanel.h/cpp** (70 + 140 lines)
- Audio settings dialog trigger
- Input/output gain sliders
- Audio/MIDI input toggles
- Real-time parameter updates

**WebServerPanel.h/cpp** (50 + 100 lines)
- Web server start/stop control
- Server status display
- Web UI launch button
- Server state management

**AudioSettingsComponent.h/cpp**
- JUCE audio device settings dialog
- Audio driver configuration
- Sample rate and buffer size settings

### Web Layer (Source/Web/)

**WebServer.h/cpp** (828 lines)
- HTTP server implementation
- Request routing
- Static file serving (HTML/CSS/JS)
- Delegates API calls to WebAPIHandler

**WebAPIHandler.h/cpp** (270 + 270 lines)
- REST API endpoint handlers
- JSON request/response processing
- Business logic for web interface
- Decoupled from HTTP protocol

## Design Patterns

### 1. Manager Pattern (Phase 2)

Separates business logic into focused manager classes:

```cpp
class PluginHostProcessor {
private:
    ConfigurationManager configManager;
    PluginManager pluginManager;
    AudioIOManager audioIOManager;
};
```

**Benefits:**
- Clear separation of concerns
- Easier unit testing
- Reduced class complexity
- Better code organization

### 2. Composition Pattern (Phase 3)

UI panels composed into MainComponent:

```cpp
class MainComponent {
private:
    std::unique_ptr<PluginControlPanel> pluginPanel;
    std::unique_ptr<AudioControlPanel> audioPanel;
    std::unique_ptr<WebServerPanel> webServerPanel;
};
```

**Benefits:**
- Modular UI components
- Independent panel development
- Simplified layout logic
- Reusable panels

### 3. Observer Pattern

Managers notify observers of state changes:

```cpp
class ConfigurationManager {
public:
    void addListener(ConfigurationListener* listener);
    void removeListener(ConfigurationListener* listener);
private:
    juce::ListenerList<ConfigurationListener> listeners;
};
```

**Benefits:**
- Decoupled communication
- Multiple observers per subject
- Event-driven architecture
- Extensible notification system

### 4. Callback Pattern

Panels communicate via `std::function` callbacks:

```cpp
class PluginControlPanel {
public:
    std::function<void()> onBrowseForPlugin;
    std::function<void(const juce::String&)> onLoadPlugin;
    std::function<void()> onShowPluginEditor;
};
```

**Benefits:**
- Loose coupling
- No inheritance required
- Flexible wiring
- Lambda-friendly

### 5. Delegation Pattern

WebServer delegates to WebAPIHandler:

```cpp
class WebServer {
private:
    std::unique_ptr<WebAPIHandler> apiHandler;

    void handleRequest() {
        response = apiHandler->handleGetStatus();
    }
};
```

**Benefits:**
- Separation of protocol and logic
- Independently testable API logic
- Protocol changes don't affect business logic

## Component Details

### PluginHostProcessor

**Responsibilities:**
- Audio processing pipeline
- Plugin hosting
- Manager coordination
- JUCE AudioProcessor implementation

**Key Methods:**
```cpp
void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&);
bool loadPlugin(const juce::File& pluginFile);
void setInputGain(float gain);
void setOutputGain(float gain);
```

**Dependencies:**
- ConfigurationManager
- PluginManager
- AudioIOManager
- juce::AudioProcessor

### WebAPIHandler

**Responsibilities:**
- REST API endpoint implementation
- JSON request/response formatting
- Business logic for web interface
- Plugin and audio control via processor

**Key Endpoints:**
```cpp
juce::String handleGetStatus();              // GET /api/status
juce::String handleGetPluginInfo();          // GET /api/plugin
juce::String handleLoadPlugin(path);         // POST /api/plugin/load
juce::String handleSetInputGain(gain);       // POST /api/audio/input-gain
juce::String handleSetOutputGain(gain);      // POST /api/audio/output-gain
```

**Dependencies:**
- PluginHostProcessor
- juce::AudioDeviceManager

### Manager Classes

#### ConfigurationManager

**State:**
- Current configuration settings
- Application preferences
- User settings

**Methods:**
```cpp
void setConfigValue(const juce::String& key, const juce::var& value);
juce::var getConfigValue(const juce::String& key);
void notifyConfigChanged();
```

#### PluginManager

**State:**
- Loaded plugin instance
- Plugin metadata
- Plugin state

**Methods:**
```cpp
bool loadPlugin(const juce::File& file);
void unloadPlugin();
bool hasPluginLoaded() const;
juce::String getLoadedPluginName() const;
juce::AudioProcessor* getLoadedPlugin();
```

#### AudioIOManager

**State:**
- Input gain (0.0 - 2.0)
- Output gain (0.0 - 2.0)
- Audio input enabled flag
- MIDI input enabled flag

**Methods:**
```cpp
void setInputGain(float gain);
void setOutputGain(float gain);
void setAudioInputEnabled(bool enabled);
void setMidiInputEnabled(bool enabled);
float getInputGain() const;
float getOutputGain() const;
```

## Data Flow

### Plugin Loading Flow

```
User clicks "Browse" button
    ↓
PluginControlPanel::onBrowseForPlugin callback
    ↓
MainComponent::browseForPlugin()
    ↓
File chooser dialog
    ↓
PluginControlPanel::setPluginPath()
    ↓
User clicks "Load Plugin"
    ↓
PluginControlPanel::onLoadPlugin callback
    ↓
MainComponent::loadPluginFromPath()
    ↓
PluginHostProcessor::loadPlugin()
    ↓
PluginManager::loadPlugin()
    ↓
JUCE plugin validation and loading
    ↓
PluginManager notifies observers
    ↓
MainComponent::timerCallback() updates UI
    ↓
PluginControlPanel::setStatusText()
```

### Web API Request Flow

```
HTTP GET /api/status
    ↓
WebServer::handleRequest()
    ↓
WebServer routes to API handler
    ↓
WebAPIHandler::handleGetStatus()
    ↓
Queries PluginHostProcessor state
    ↓
PluginManager::hasPluginLoaded()
    ↓
Formats JSON response
    ↓
Returns to WebServer
    ↓
WebServer sends HTTP response
    ↓
Browser receives JSON
```

### Audio Processing Flow

```
Audio driver callback
    ↓
juce::AudioDeviceManager
    ↓
juce::AudioProcessorPlayer
    ↓
PluginHostProcessor::processBlock()
    ↓
AudioIOManager::applyInputGain()
    ↓
Check if audio input enabled
    ↓
PluginManager::getLoadedPlugin()
    ↓
Plugin processes audio
    ↓
AudioIOManager::applyOutputGain()
    ↓
Return processed audio to driver
```

## Directory Structure

```
Source/
├── Core/                          # Business logic layer
│   ├── PluginHostProcessor.h/cpp  # Main processor
│   ├── ConfigurationManager.h/cpp # Config management
│   ├── PluginManager.h/cpp        # Plugin operations
│   └── AudioIOManager.h/cpp       # Audio I/O control
│
├── UI/                            # User interface layer
│   ├── MainComponent.h/cpp        # Main window
│   ├── PluginControlPanel.h/cpp   # Plugin controls
│   ├── AudioControlPanel.h/cpp    # Audio controls
│   ├── WebServerPanel.h/cpp       # Web server controls
│   └── AudioSettingsComponent.h/cpp
│
├── Web/                           # Web interface layer
│   ├── WebServer.h/cpp            # HTTP server
│   └── WebAPIHandler.h/cpp        # REST API logic
│
└── Main.cpp                       # Application entry point
```

## Testing Strategy

### Unit Testing

Each component can be tested in isolation:

**WebAPIHandler Tests:**
```cpp
// Mock PluginHostProcessor and AudioDeviceManager
// Test each endpoint handler independently
// Verify JSON response format
```

**Manager Tests:**
```cpp
// Test ConfigurationManager state changes
// Test PluginManager load/unload logic
// Test AudioIOManager parameter validation
```

**Panel Tests:**
```cpp
// Test callback invocation
// Test state update methods
// Test UI state consistency
```

### Integration Testing

Test component interactions:

- MainComponent → Panels → Callbacks
- WebAPIHandler → PluginHostProcessor → Managers
- WebServer → WebAPIHandler → Business Logic

### Test Structure

```
Source/Tests/
├── Core/
│   ├── ConfigurationManagerTests.cpp
│   ├── PluginManagerTests.cpp
│   └── AudioIOManagerTests.cpp
├── Web/
│   ├── WebAPIHandlerTests.cpp
│   └── WebServerTests.cpp
└── UI/
    ├── PluginControlPanelTests.cpp
    ├── AudioControlPanelTests.cpp
    └── WebServerPanelTests.cpp
```

## Thread Safety

### Real-time Audio Thread

- **PluginHostProcessor::processBlock()** runs on real-time audio thread
- No memory allocation in processBlock()
- Lock-free parameter access via atomics
- Message passing for non-RT operations

### UI Thread

- All UI updates on message thread
- Timer-based polling for state changes
- Async callbacks for long operations

### Web Server Thread

- Separate thread for HTTP server
- Thread-safe access to processor state
- No blocking operations on audio thread

## Performance Considerations

### Memory Management

- Pre-allocated buffers in audio path
- Smart pointers for RAII
- No allocations in processBlock()

### Lock-free Design

- Atomic variables for RT-safe parameters
- Message queue for RT → UI communication
- No mutexes in audio thread

### Optimization

- Panel composition reduces object count
- Delegation pattern reduces code duplication
- Manager pattern improves cache locality

## Future Extensions

### Planned Improvements

1. **Plugin Chain**: Multiple plugins in series
2. **Preset Management**: Save/load plugin states
3. **MIDI Routing**: Flexible MIDI I/O
4. **Automation**: Parameter automation recording
5. **Additional Formats**: AU, AAX, CLAP support

### Architectural Extensibility

The current architecture supports these extensions:

- **PluginChainManager**: New manager in Core layer
- **PresetManager**: New manager in Core layer
- **MIDIRoutingManager**: Extension of AudioIOManager
- **AutomationManager**: New manager with timeline

## Refactoring History

### Phase 1: Initial Implementation
- Basic VST hosting
- Simple UI
- Monolithic design

### Phase 2: Manager Pattern
- Extracted ConfigurationManager
- Extracted PluginManager
- Extracted AudioIOManager
- Reduced PluginHostProcessor complexity

### Phase 3: Modular Architecture
- Created WebAPIHandler (delegated from WebServer)
- Created UI Panels (PluginControlPanel, AudioControlPanel, WebServerPanel)
- Organized files into functional directories (Core/, UI/, Web/)
- Reduced MainComponent from 446 to 322 lines (-28%)
- Reduced WebServer from 1,030 to 828 lines (-20%)

## Code Metrics

### Phase 3 Results

**File Organization:**
- Core components: 4 files
- UI components: 8 files
- Web components: 4 files

**Line Counts:**
- WebServer.cpp: 1,030 → 828 lines (-20%)
- MainComponent.cpp: 446 → 322 lines (-28%)
- New modular components: 1,545 lines

**Complexity Reduction:**
- MainComponent constructor: 115 → 55 lines (-52%)
- MainComponent resized(): 65 → 20 lines (-69%)
- Code duplication eliminated: 519 lines

**Test Coverage Goals:**
- Core managers: 80%+
- API handlers: 90%+
- UI panels: 50%+

## References

- [JUCE Framework Documentation](https://juce.com/learn/documentation)
- [WEB_INTERFACE.md](../manual/WEB_INTERFACE.md) - Web API specification
