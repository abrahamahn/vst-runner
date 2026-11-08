# VST Runner

A minimal, lightweight, cross-platform VST plugin host application for Windows and macOS.

## Features

- 🎵 **Plugin Format Support**: VST2, VST3 (primary), with extensibility for AAX, AU, and CLAP
- 🖥️ **Cross-Platform**: Windows and macOS compatible
- 🌐 **Web Interface**: Control your VST plugins from any browser (localhost)
- 🎛️ **Audio Driver Integration**: Full ASIO support (Windows), CoreAudio (macOS)
- 🎹 **Plugin Types**: Both audio FX and MIDI instrument plugins
- 📁 **Easy Plugin Loading**: Native file browser or manual path input
- ⚡ **Lightweight**: Minimal resource footprint
- 🔊 **Full Audio I/O**: Input and output signal routing with volume control
- 🔌 **Dual Mode**: Use as desktop app or web-based controller

## Architecture

Built with [JUCE](https://juce.com/) - the industry-standard C++ framework for audio applications.

VST Runner features a modular, layered architecture designed for maintainability and testability:

### Core Layer
- **PluginHostProcessor**: Main audio processor with real-time audio/MIDI routing
- **ConfigurationManager**: Application configuration and state management
- **PluginManager**: Plugin lifecycle and loading operations
- **AudioIOManager**: Audio/MIDI I/O control and gain management

### UI Layer
- **MainComponent**: Main application window (composition of modular panels)
- **PluginControlPanel**: Plugin browsing, loading, and editor controls
- **AudioControlPanel**: Audio settings, gain sliders, and I/O toggles
- **WebServerPanel**: Web server control and status

### Web Layer
- **WebServer**: HTTP server for browser-based control
- **WebAPIHandler**: REST API business logic (decoupled from HTTP protocol)

For detailed architecture documentation, see [docs/dev/ARCHITECTURE.md](docs/dev/ARCHITECTURE.md)

## Building

For detailed build instructions, see [docs/manual/BUILD.md](docs/manual/BUILD.md).

**Looking for single-file executable or installer packages?** See [docs/manual/BUILD_DISTRIBUTION.md](docs/manual/BUILD_DISTRIBUTION.md) for:
- **Portable single-file executables** (like GHelper - just download and run!)
- **Installer packages** (Windows .exe, macOS .dmg, Linux .deb/.rpm)
- **Platform-specific build scripts** for easy compilation

### Quick Start

**One-command setup** (recommended for contributors):

```bash
# Linux/macOS
./setup.sh

# Windows (PowerShell)
.\setup.ps1
```

**Manual build:**

```bash
# Clone the repository
git clone <repository-url>
cd vst-runner

# Initialize and update JUCE submodule
git submodule update --init --recursive

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build . --config Release

# On Windows, the executable will be in build/VSTRunner_artefacts/Release/
# On macOS, the app bundle will be in build/VSTRunner_artefacts/Release/
```

**Using Makefile** (Linux/macOS):

```bash
make setup    # Initial setup
make build    # Build project
make test     # Run tests
make help     # Show all commands
```

### Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler:
  - Windows: Visual Studio 2019 or later
  - macOS: Xcode 11 or later
- Git

## Usage

For detailed setup and usage instructions, see [SETUP.md](SETUP.md).

### Desktop Mode (Quick Start)

1. **Launch the application**
2. **Configure Audio Settings**: Click "Audio Settings" to select your audio interface and configure ASIO drivers
3. **Load a Plugin**:
   - Click "Browse" to open the file browser
   - Or enter the plugin path manually
   - Select a `.dll` (VST2), `.vst3` (VST3 folder), or plugin file
4. **Use the Plugin**: The plugin's editor window will open automatically
5. **Audio/MIDI I/O**:
   - Audio input/output is automatically routed
   - MIDI input is available for instrument plugins
   - Adjust input/output levels as needed

### Web Mode

1. **Start Web Server**: Click "Start Web Server" in the desktop application
2. **Open Browser**: Click "Open Web UI" or navigate to `http://localhost:8080`
3. **Control from Browser**:
   - Load plugins by entering the full path
   - Adjust input/output gain with sliders
   - Enable/disable audio and MIDI input
   - Monitor plugin status in real-time
4. **Access from Any Device**: Connect from any device on your local network (optional)

**Note**: The web interface runs locally and communicates with the native application. VST plugins are loaded and processed natively on your machine for best performance.

## Supported Plugin Formats

| Format | Windows | macOS | Status |
|--------|---------|-------|--------|
| VST2   | ✅ .dll | ✅ .vst | Fully Supported |
| VST3   | ✅ .vst3 | ✅ .vst3 | Fully Supported |
| AU     | ❌ | 🔄 | Planned |
| AAX    | 🔄 | 🔄 | Planned |
| CLAP   | 🔄 | 🔄 | Planned |

## Technical Details

- **Framework**: JUCE 7.x
- **Language**: C++17
- **Build System**: CMake
- **Audio Processing**: 32-bit float, configurable sample rate and buffer size
- **Threading**: Real-time audio thread with lock-free message passing
- **Plugin Hosting**: JUCE AudioPluginHost components

## License

MIT License - see [LICENSE](LICENSE) file for details.

**Important:** This project uses JUCE, which is licensed under GPLv3. If you distribute this software:
- You must comply with GPLv3 terms, OR
- Purchase a commercial JUCE license from https://juce.com/

See [JUCE Licensing](https://juce.com/juce-7-licence/) for more information.

## Testing

VST Runner includes a comprehensive unit test suite using JUCE's built-in test framework.

### Running Tests

```bash
# Build tests (enabled by default)
cmake .. -DBUILD_TESTS=ON
cmake --build . --target VSTRunnerTests

# Run all tests
./VSTRunnerTests_artefacts/Debug/VSTRunnerTests

# Run tests by category
./VSTRunnerTests_artefacts/Debug/VSTRunnerTests --category Core
./VSTRunnerTests_artefacts/Debug/VSTRunnerTests --category Web

# Run specific test
./VSTRunnerTests_artefacts/Debug/VSTRunnerTests --test AudioIOManager
```

### Test Coverage

- **Core Layer**: ConfigurationManager, PluginManager, AudioIOManager (80%+ coverage goal)
- **Web Layer**: WebAPIHandler endpoints and JSON handling (90%+ coverage goal)
- **UI Layer**: Panel callbacks and state management (50%+ coverage goal)

For more details, see [docs/dev/ARCHITECTURE.md#testing-strategy](docs/dev/ARCHITECTURE.md#testing-strategy)

## Contributing

Contributions are welcome! Please read our [CONTRIBUTING.md](CONTRIBUTING.md) for:
- Development workflow and setup
- Coding standards and style guide
- Testing guidelines
- Pull request process
- Commit message guidelines

## Troubleshooting

### Windows
- Ensure ASIO drivers are installed for your audio interface
- Run as Administrator if plugin scanning fails
- Check Windows Defender isn't blocking plugin DLLs

### macOS
- Grant microphone and audio permissions in System Preferences
- For VST2 plugins, check `/Library/Audio/Plug-Ins/VST/`
- For VST3 plugins, check `/Library/Audio/Plug-Ins/VST3/`

## Documentation

### User Manuals (`docs/manual/`)
- **[BUILD.md](docs/manual/BUILD.md)** - Detailed build instructions for all platforms
- **[BUILD_DISTRIBUTION.md](docs/manual/BUILD_DISTRIBUTION.md)** - Single-file executables and installer packages
- **[BUILD_WINDOWS_INSTRUCTIONS.md](docs/manual/BUILD_WINDOWS_INSTRUCTIONS.md)** - Windows-specific build guide
- **[BUILD_CONFIGURATION.md](docs/manual/BUILD_CONFIGURATION.md)** - Build configuration options
- **[SETUP.md](docs/manual/SETUP.md)** - Complete usage guide and setup instructions
- **[WEB_INTERFACE.md](docs/manual/WEB_INTERFACE.md)** - Web interface API and architecture
- **[GETTING_STARTED.md](docs/manual/GETTING_STARTED.md)** - Beginner's guide to the tech stack

### Developer Documentation (`docs/dev/`)
- **[ARCHITECTURE.md](docs/dev/ARCHITECTURE.md)** - Complete architecture documentation and design patterns

### Project Documentation (root)
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - How to contribute to this project
- **[CHANGELOG.md](CHANGELOG.md)** - Version history and release notes
- **[LICENSE](LICENSE)** - MIT License

## Credits

Built with [JUCE](https://github.com/juce-framework/JUCE) by Avid/ROLI.
