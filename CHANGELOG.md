# Changelog

All notable changes to VST Runner will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Comprehensive test suite with 32 unit tests
- Performance benchmark infrastructure
- Docker support for Linux builds
- GitHub Actions CI/CD workflow
- Pre-commit hooks for code formatting
- Issue and PR templates

### Changed
- Reorganized documentation into `/docs` structure
  - Developer docs in `/docs/dev/`
  - User manuals in `/docs/manual/`

## [1.0.0] - 2025-11-10

### Added
- **Core Features**
  - VST2 and VST3 plugin hosting
  - Cross-platform support (Windows, macOS, Linux)
  - Real-time audio processing with configurable I/O
  - Web-based remote control interface
  - Plugin editor window hosting
  - Audio device configuration (ASIO, CoreAudio, ALSA)
  - Input/output gain control (0.0 - 2.0x)
  - Audio and MIDI input enable/disable
  - Plugin path browser and manual entry
  - REST API for programmatic control

- **Architecture**
  - Modular layered architecture (Core, UI, Web)
  - Manager pattern for business logic separation
  - Composition-based UI with reusable panels
  - Observer pattern for state notifications
  - Callback-based loose coupling

- **Testing**
  - Unit test framework using JUCE UnitTest
  - 32 test cases across Core and Web layers
  - Test runner with category filtering
  - Coverage goals: Core 80%+, Web 90%+, UI 50%+

- **Documentation**
  - Complete architecture documentation (450+ lines)
  - API documentation for all public interfaces
  - Build guides for all platforms
  - Setup and usage manuals
  - Web interface API documentation
  - Refactoring history and design decisions

### Phase 3 - Modular Architecture (2025-11-10)

#### Part 1: File Organization
- Organized source files into functional directories
  - `Source/Core/` - Business logic managers
  - `Source/UI/` - User interface components
  - `Source/Web/` - Web server and API
  - `Source/Tests/` - Unit test suite
- Extracted WebAPIHandler from WebServer (270 lines)
- Reduced files in Source/ root from 10 to 2 (-80%)

#### Part 2: Component Modularization
- Created modular UI panels
  - `PluginControlPanel` (210 lines) - Plugin browsing and loading
  - `AudioControlPanel` (205 lines) - Audio settings and gain control
  - `WebServerPanel` (130 lines) - Web server control
- Refactored MainComponent
  - Reduced from 446 to 322 lines (-28%)
  - Constructor: 115 → 55 lines (-52%)
  - Layout method: 65 → 20 lines (-69%)
- Integrated WebAPIHandler into WebServer
  - Reduced WebServer from 1,030 to 828 lines (-20%)
  - Separated HTTP protocol from business logic
  - Enabled independent API testing

#### Part 3: Testing Infrastructure
- Created comprehensive test suite (820 lines)
  - WebAPIHandlerTests (11 test cases, 280 lines)
  - AudioIOManagerTests (11 test cases, 210 lines)
  - PluginManagerTests (10 test cases, 180 lines)
- Implemented test runner with category filtering
- Added BUILD_TESTS CMake option (ON by default)
- Created ARCHITECTURE.md (450+ lines)

**Metrics:**
- Code duplication: Eliminated 326 lines
- Organization: 1 → 4 functional directories (+300%)
- Testability: None → High (32 test cases)
- Documentation: ~500 → ~2,000 lines

### Phase 2 - Manager Pattern (2025-11-09)

#### Added
- **ConfigurationManager** (145 lines)
  - Configuration state management
  - Observer pattern for config changes
  - Thread-safe configuration access

- **PluginManager** (180 lines)
  - Plugin lifecycle management
  - Plugin validation and metadata
  - Observer notifications for state changes

- **AudioIOManager** (135 lines)
  - Audio/MIDI I/O control
  - Gain management with thread-safe atomics
  - Real-time parameter changes

#### Changed
- Refactored PluginHostProcessor
  - Extracted business logic into three managers
  - Reduced complexity and improved testability
  - Clear separation of concerns

**Metrics:**
- PluginHostProcessor: Reduced to 370 lines (manageable)
- New manager classes: 460 lines total
- Code organization: Significantly improved

### Phase 1 - Initial Implementation (2025-11-08)

#### Added
- Basic VST plugin hosting
- Simple desktop UI with JUCE
- Audio device management
- Plugin editor window
- Web server with REST API
- HTML/CSS/JS web interface

#### Features
- Load VST2 (.dll, .vst) and VST3 (.vst3) plugins
- Real-time audio processing
- Plugin editor hosting
- Web-based remote control
- Audio settings configuration
- Input/output gain control

## API Changes

### [1.0.0]

#### Web API Endpoints

**Plugin Operations:**
- `GET /api/status` - Get current plugin and server status
- `GET /api/plugin` - Get loaded plugin information
- `POST /api/plugin/load` - Load plugin by path
- `POST /api/plugin/unload` - Unload current plugin
- `POST /api/plugin/scan` - Scan directory for plugins

**Audio Control:**
- `GET /api/audio/settings` - Get audio I/O settings
- `POST /api/audio/input-gain` - Set input gain (0.0 - 2.0)
- `POST /api/audio/output-gain` - Set output gain (0.0 - 2.0)
- `POST /api/audio/input` - Enable/disable audio input
- `POST /api/audio/midi` - Enable/disable MIDI input

**Response Format:**
```json
{
  "success": true,
  "data": { ... }
}
```

**Error Format:**
```json
{
  "error": "Error message"
}
```

## Breaking Changes

### [1.0.0]
- Initial release - no breaking changes

## Migration Guide

### From Development to 1.0.0

No migration needed. First stable release.

## Known Issues

### [1.0.0]
- Web server port (8080) is not configurable
- Plugin scanning is synchronous (may block UI)
- No plugin preset management
- No plugin chain support
- No MIDI routing customization

## Dependencies

### [1.0.0]
- JUCE 7.x
- CMake 3.15+
- C++17 compiler

## Platform Support

### [1.0.0]
- ✅ Windows 10/11 (MSVC 2019+)
- ✅ macOS 11+ (Xcode 11+)
- ✅ Linux (GCC 9+, Clang 10+)

### Audio Drivers
- Windows: ASIO, WASAPI, DirectSound
- macOS: CoreAudio
- Linux: ALSA, JACK

## Acknowledgments

- Built with [JUCE](https://juce.com/) framework
- VST SDK by Steinberg

---

## Release Notes Format

Each release includes:
- **Version Number**: Semantic versioning (MAJOR.MINOR.PATCH)
- **Release Date**: YYYY-MM-DD format
- **Changes**: Categorized as Added, Changed, Deprecated, Removed, Fixed, Security
- **Breaking Changes**: Highlighted separately
- **Migration Guide**: For breaking changes

[Unreleased]: https://github.com/abrahamahn/vst-runner/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/abrahamahn/vst-runner/releases/tag/v1.0.0
