# Refactoring Plan for VST Runner

## Current Analysis

### Code Metrics
- **Total Lines**: ~2,155 lines
- **Largest File**: WebServer.cpp (968 lines) - mostly embedded HTML
- **MainComponent.cpp**: 445 lines - too many responsibilities

### Identified Issues

#### 1. **Separation of Concerns**

**MainComponent Violations**:
```
MainComponent currently does:
- UI rendering and layout
- Plugin management (loading, unloading)
- Audio device management
- Web server lifecycle management
- Plugin editor window management
- Audio/MIDI routing control
- File browsing logic
```

**Problem**: Violates Single Responsibility Principle (SRP). Hard to test and maintain.

#### 2. **Code Organization**

**Current Structure**:
```
Source/
├── Main.cpp                      (Entry point - OK)
├── MainComponent.{h,cpp}         (Too large, multiple concerns)
├── PluginHostProcessor.{h,cpp}   (Mixing plugin loading + audio processing)
├── AudioSettingsComponent.{h,cpp} (OK - focused)
└── WebServer.{h,cpp}             (Embedded HTML - maintainability issue)
```

**Issues**:
- No clear layered architecture
- Business logic mixed with UI code
- No dedicated managers/controllers
- No configuration management

#### 3. **Scalability Concerns**

**Hard to Add Features**:
- New plugin formats → Modify PluginHostProcessor
- Multiple plugins → Major MainComponent refactoring
- Plugin presets → No clear place to add
- Audio routing → Tightly coupled with UI

**No Extensibility Points**:
- No plugin interfaces/abstract base classes
- Hard-coded dependencies
- Tight coupling between components

#### 4. **Maintainability Issues**

**WebServer Embedded HTML**:
- 700+ lines of HTML/CSS/JS in C++ string
- Hard to edit and maintain
- No syntax highlighting for web code
- Difficult for web developers to contribute

**Error Handling**:
- Inconsistent error handling patterns
- Mix of AlertWindow and return values
- No centralized error management

**Configuration**:
- Hard-coded values (port 8080)
- No user preferences/settings persistence
- No configuration file support

#### 5. **Testability**

**Current Issues**:
- Hard to unit test (everything is coupled)
- No dependency injection
- No mock-able interfaces
- UI and logic tightly bound

---

## Proposed Refactoring

### Phase 1: Extract Business Logic (High Priority)

#### 1.1 Create PluginManager
**Purpose**: Centralize all plugin-related operations

```cpp
class PluginManager
{
public:
    bool loadPlugin(const juce::File& pluginFile);
    void unloadPlugin();
    bool hasPluginLoaded() const;
    juce::String getLoadedPluginName() const;
    AudioPluginInstance* getPlugin() const;

    // Plugin scanning
    juce::Array<PluginDescription> scanDirectory(const juce::File& directory);

    // Listeners for state changes
    class Listener {
        virtual void pluginLoaded(const juce::String& name) = 0;
        virtual void pluginUnloaded() = 0;
        virtual void pluginLoadFailed(const juce::String& error) = 0;
    };
};
```

**Benefits**:
- Single source of truth for plugin state
- Easier to add plugin presets
- Can add plugin scanning/validation
- Testable without UI

#### 1.2 Create AudioIOManager
**Purpose**: Manage audio/MIDI routing and gain controls

```cpp
class AudioIOManager
{
public:
    void setInputGain(float gain);
    void setOutputGain(float gain);
    float getInputGain() const;
    float getOutputGain() const;

    void setAudioInputEnabled(bool enabled);
    void setMidiInputEnabled(bool enabled);
    bool isAudioInputEnabled() const;
    bool isMidiInputEnabled() const;

    // Audio device management
    void setAudioDevice(AudioDeviceManager& deviceManager);
};
```

**Benefits**:
- Decouples audio routing from UI
- Easier to add audio metering
- Can add preset gain settings
- Better for automation

#### 1.3 Create ConfigurationManager
**Purpose**: Centralized configuration and settings

```cpp
class ConfigurationManager
{
public:
    // Server settings
    int getWebServerPort() const;
    void setWebServerPort(int port);

    // Last used paths
    juce::File getLastPluginDirectory() const;
    void setLastPluginDirectory(const juce::File& dir);

    // Audio settings
    void saveAudioSettings(const AudioDeviceSetup& setup);
    AudioDeviceSetup loadAudioSettings();

    // Persistence
    void save();
    void load();
};
```

**Benefits**:
- User preferences persist
- Easy to add new settings
- Configuration in one place
- Can use JSON/XML files

### Phase 2: Improve File Organization (High Priority)

#### 2.1 Restructure Directories

**Proposed Structure**:
```
Source/
├── Core/                         # Business logic (no UI)
│   ├── PluginManager.{h,cpp}
│   ├── AudioIOManager.{h,cpp}
│   ├── ConfigurationManager.{h,cpp}
│   └── PluginHostProcessor.{h,cpp}
│
├── UI/                           # User interface components
│   ├── MainComponent.{h,cpp}     (Refactored - UI only)
│   ├── AudioSettingsComponent.{h,cpp}
│   ├── PluginBrowserComponent.{h,cpp}  (New)
│   └── ControlPanelComponent.{h,cpp}   (New)
│
├── Web/                          # Web interface
│   ├── WebServer.{h,cpp}         (Refactored)
│   ├── WebAPIHandler.{h,cpp}     (New - API logic)
│   └── Resources/
│       └── index.html            (Extracted HTML)
│
├── Utils/                        # Utilities
│   ├── ErrorHandler.{h,cpp}      (New)
│   └── Logger.{h,cpp}            (New - optional)
│
└── Main.cpp
```

**Benefits**:
- Clear separation of concerns
- Easy to find files
- Better for IDE navigation
- Scalable structure

#### 2.2 Extract Web UI HTML

**Create**: `Source/Web/Resources/index.html`

**Benefits**:
- Web developers can edit HTML directly
- Syntax highlighting and validation
- Easier to update UI
- Can use build system to embed if needed

### Phase 3: Refactor MainComponent (Medium Priority)

#### 3.1 Extract Sub-Components

**Current MainComponent** has too many UI elements. Break down:

**ControlPanelComponent**:
```cpp
// Gain controls, input toggles, etc.
class ControlPanelComponent : public Component
{
    Slider inputGainSlider;
    Slider outputGainSlider;
    ToggleButton audioInputToggle;
    ToggleButton midiInputToggle;

    // Listener interface for MainComponent
    class Listener {
        virtual void gainChanged(float input, float output) = 0;
        virtual void inputStateChanged(bool audio, bool midi) = 0;
    };
};
```

**PluginBrowserComponent**:
```cpp
// Plugin path entry and loading
class PluginBrowserComponent : public Component
{
    TextEditor pluginPathEditor;
    TextButton browseButton;
    TextButton loadButton;

    class Listener {
        virtual void pluginLoadRequested(const juce::File& file) = 0;
    };
};
```

**Benefits**:
- MainComponent becomes a coordinator
- Each component is reusable
- Easier to test individual components
- Better encapsulation

### Phase 4: Improve WebServer Architecture (Medium Priority)

#### 4.1 Separate API Logic

**Current**: WebServer has both HTTP handling AND business logic

**Proposed**: Create WebAPIHandler

```cpp
class WebAPIHandler
{
public:
    WebAPIHandler(PluginManager* pm, AudioIOManager* am);

    juce::var handleGetStatus();
    juce::var handleGetPluginInfo();
    juce::var handleLoadPlugin(const juce::var& request);
    juce::var handleSetInputGain(const juce::var& request);
    // etc.

private:
    PluginManager* pluginManager;
    AudioIOManager* audioIOManager;
};
```

**WebServer becomes**:
```cpp
class WebServer
{
    // Only HTTP/networking concerns
    void run() override;
    juce::String routeRequest(const juce::String& method,
                               const juce::String& path,
                               const juce::String& body);
private:
    std::unique_ptr<WebAPIHandler> apiHandler;
};
```

**Benefits**:
- API logic testable without HTTP
- Can add WebSocket server easily
- Better error handling
- Cleaner code

#### 4.2 Extract HTML Resources

**Use JUCE BinaryData** to embed HTML:

In CMakeLists.txt:
```cmake
juce_add_binary_data(WebResources
    SOURCES
        Source/Web/Resources/index.html
        Source/Web/Resources/style.css
        Source/Web/Resources/app.js
)

target_link_libraries(VSTRunner PRIVATE WebResources)
```

**Benefits**:
- Separate files for web development
- Can minify/optimize separately
- Still embedded in binary (no external files)
- Easy to update

### Phase 5: Add Error Handling (Low Priority)

#### 5.1 Create ErrorHandler

```cpp
class ErrorHandler
{
public:
    enum class Severity { Info, Warning, Error, Critical };

    static void handleError(Severity severity,
                           const juce::String& message,
                           const juce::String& context);

    static void showUserError(const juce::String& title,
                             const juce::String& message);

    // Optional: Error logging
    static void logError(const juce::String& error);
};
```

**Benefits**:
- Consistent error handling
- Easy to add logging
- Can add error reporting
- Better user experience

### Phase 6: Improve Testability (Low Priority)

#### 6.1 Add Interfaces

```cpp
// Abstract interface for plugin hosting
class IPluginHost
{
public:
    virtual ~IPluginHost() = default;
    virtual bool loadPlugin(const juce::File& file) = 0;
    virtual void unloadPlugin() = 0;
    virtual bool hasPluginLoaded() const = 0;
};

// PluginHostProcessor implements IPluginHost
// Can create MockPluginHost for testing
```

**Benefits**:
- Unit testing without real plugins
- Dependency injection
- Better architecture
- Easier to maintain

---

## Implementation Priority

### Immediate (High ROI, Low Risk)

1. **Extract HTML from WebServer** → External file
   - Impact: High (maintainability)
   - Risk: Low (just file extraction)
   - Time: 1-2 hours

2. **Create ConfigurationManager** → User settings
   - Impact: High (UX improvement)
   - Risk: Low (additive change)
   - Time: 2-3 hours

3. **Reorganize directory structure** → Core/UI/Web folders
   - Impact: High (organization)
   - Risk: Medium (need to update includes)
   - Time: 1-2 hours

### Short-term (High Impact)

4. **Create PluginManager** → Extract plugin logic
   - Impact: Very High (scalability)
   - Risk: Medium (refactor existing code)
   - Time: 4-6 hours

5. **Create AudioIOManager** → Extract audio routing
   - Impact: High (testability)
   - Risk: Low (mostly extraction)
   - Time: 2-3 hours

6. **Refactor MainComponent** → Split into sub-components
   - Impact: Very High (maintainability)
   - Risk: Medium (UI refactoring)
   - Time: 4-6 hours

### Long-term (Nice to Have)

7. **Create WebAPIHandler** → Separate API logic
   - Impact: Medium (testability)
   - Risk: Low (extraction)
   - Time: 3-4 hours

8. **Add ErrorHandler** → Centralized error handling
   - Impact: Medium (UX)
   - Risk: Low (additive)
   - Time: 2-3 hours

9. **Add Interfaces** → Improve testability
   - Impact: Medium (testing)
   - Risk: Low (additive)
   - Time: 2-4 hours

---

## Specific Code Smells Found

### 1. Embedded HTML in C++ (WebServer.cpp:247-968)
```cpp
// 721 lines of HTML in C++ string - BAD
juce::String WebServer::getIndexHTML()
{
    return R"HTML(
    <!DOCTYPE html>
    ...
    )HTML";
}
```

**Fix**: Extract to separate HTML file, use JUCE BinaryData

### 2. MainComponent God Class
```cpp
// MainComponent.h - too many responsibilities
class MainComponent : public Component, private Timer
{
    // UI elements
    // Audio management
    // Plugin management
    // Web server management
    // File browser logic
    // Editor window management
    // ...all in one class
};
```

**Fix**: Split into PluginManager, AudioIOManager, sub-components

### 3. Hard-coded Values
```cpp
// WebServer.cpp:413
if (webServer->start (8080))  // Hard-coded port

// MainComponent.cpp:138
setSize (700, 600);  // Hard-coded window size
```

**Fix**: ConfigurationManager with user preferences

### 4. No Error Abstraction
```cpp
// Inconsistent error handling:
juce::AlertWindow::showMessageBoxAsync(...);  // Sometimes
return errorResponse("...");                   // Sometimes
DBG("...");                                    // Sometimes
```

**Fix**: ErrorHandler class with consistent patterns

### 5. Tight Coupling
```cpp
// MainComponent directly instantiates everything
pluginProcessor = std::make_unique<PluginHostProcessor>();
webServer = std::make_unique<WebServer>(...);
```

**Fix**: Dependency injection, interfaces

---

## Benefits Summary

| Refactoring | Maintainability | Testability | Scalability | Performance |
|-------------|----------------|-------------|-------------|-------------|
| Extract HTML | ⭐⭐⭐⭐⭐ | - | - | - |
| PluginManager | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | - |
| AudioIOManager | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ | - |
| ConfigManager | ⭐⭐⭐⭐⭐ | ⭐⭐ | ⭐⭐⭐ | - |
| Directory structure | ⭐⭐⭐⭐⭐ | ⭐⭐ | ⭐⭐⭐⭐ | - |
| Split MainComponent | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | - |
| WebAPIHandler | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | - |
| ErrorHandler | ⭐⭐⭐⭐ | ⭐⭐ | ⭐⭐ | - |
| Interfaces | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | - |

---

## Migration Strategy

### Option 1: Big Bang (Not Recommended)
- Refactor everything at once
- High risk
- Hard to test incrementally

### Option 2: Strangler Fig Pattern (Recommended)
- Implement new architecture alongside old
- Gradually migrate functionality
- Old code and new code coexist
- Low risk, incremental progress

**Example**:
1. Create PluginManager
2. MainComponent uses BOTH old code AND PluginManager
3. Gradually move logic to PluginManager
4. Remove old code when PluginManager is complete
5. Repeat for other managers

### Option 3: Feature Freeze + Refactor
- Stop new features
- Dedicated refactoring sprint
- Medium risk

**Recommended: Option 2 (Strangler Fig)**

---

## Next Steps

### Immediate Actions:
1. Review this refactoring plan
2. Prioritize which refactorings to do first
3. Create feature branches for each refactoring
4. Implement highest priority items
5. Test thoroughly
6. Merge incrementally

### Questions to Answer:
1. Do we want unit tests? (Will guide interface design)
2. Do we need plugin presets? (Affects PluginManager design)
3. Multiple plugin support planned? (Affects architecture significantly)
4. Do we want configuration files or just in-memory settings?
5. Should we support plugin scanning/browsing?

---

## Conclusion

The current codebase is functional but has scalability and maintainability issues. The proposed refactoring will:

✅ Improve separation of concerns
✅ Make code more testable
✅ Enable easier feature additions
✅ Improve maintainability
✅ Better file organization
✅ Reduce coupling

**Estimated Total Effort**: 25-40 hours for full refactoring
**Recommended First Steps**: Extract HTML, ConfigManager, directory reorg (5-6 hours)

Would you like me to proceed with implementing specific refactorings?
