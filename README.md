# VST Runner

A minimal, lightweight, cross-platform VST plugin host application for Windows and macOS.

## Features

- 🎵 **Plugin Format Support**: VST2, VST3 (primary), with extensibility for AAX, AU, and CLAP
- 🖥️ **Cross-Platform**: Windows and macOS compatible
- 🎛️ **Audio Driver Integration**: Full ASIO support (Windows), CoreAudio (macOS)
- 🎹 **Plugin Types**: Both audio FX and MIDI instrument plugins
- 📁 **Easy Plugin Loading**: Native file browser or manual path input
- ⚡ **Lightweight**: Minimal resource footprint
- 🔊 **Full Audio I/O**: Input and output signal routing with volume control

## Architecture

Built with [JUCE](https://juce.com/) - the industry-standard C++ framework for audio applications.

### Components

- **Main Application Window**: Plugin browser and control interface
- **Audio Engine**: JUCE AudioDeviceManager for ASIO/driver integration
- **Plugin Host**: VST2/VST3 plugin loading and hosting
- **Audio Processor**: Real-time audio/MIDI routing and processing
- **Plugin Scanner**: Automatic plugin discovery and validation

## Building

For detailed build instructions, see [BUILD.md](BUILD.md).

### Quick Start

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

### Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler:
  - Windows: Visual Studio 2019 or later
  - macOS: Xcode 11 or later
- Git

## Usage

For detailed setup and usage instructions, see [SETUP.md](SETUP.md).

### Quick Start

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

## Contributing

Contributions are welcome! Please ensure code follows the existing style and includes appropriate tests.

## Troubleshooting

### Windows
- Ensure ASIO drivers are installed for your audio interface
- Run as Administrator if plugin scanning fails
- Check Windows Defender isn't blocking plugin DLLs

### macOS
- Grant microphone and audio permissions in System Preferences
- For VST2 plugins, check `/Library/Audio/Plug-Ins/VST/`
- For VST3 plugins, check `/Library/Audio/Plug-Ins/VST3/`

## Credits

Built with [JUCE](https://github.com/juce-framework/JUCE) by Avid/ROLI.
