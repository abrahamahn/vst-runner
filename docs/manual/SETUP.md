# Setup and Usage Guide

This guide will help you set up and use VST Runner after building it.

## First Launch

When you first launch VST Runner, you'll see the main interface with:

- **Plugin Path** field and browser
- **Audio Settings** button
- **Input/Output gain** controls
- **Audio/MIDI input toggles**

## Audio Configuration

### Step 1: Configure Audio Settings

1. Click the **"Audio Settings"** button
2. In the Audio Settings window:

   **Select Audio Device Type:**
   - **Windows**: Choose "ASIO" for best performance (requires ASIO drivers)
     - Alternatives: "Windows Audio" (WASAPI), "DirectSound"
   - **macOS**: "CoreAudio" (automatically selected)
   - **Linux**: "ALSA" or "JACK"

   **Select Audio Device:**
   - Choose your audio interface from the dropdown
   - Built-in options: "Default", "Built-in Audio"
   - External: Your USB/Thunderbolt audio interface name

   **Configure Sample Rate:**
   - Common values: 44100 Hz, 48000 Hz, 96000 Hz
   - Higher = better quality, more CPU usage
   - Recommended: 48000 Hz

   **Configure Buffer Size:**
   - Lower = less latency, more CPU usage
   - Higher = more latency, less CPU usage
   - Recommended: 256 or 512 samples
   - For live performance: 128 or 256
   - For mixing: 512 or 1024

   **Select Active Channels:**
   - Enable the input/output channels you want to use
   - Typically: Stereo (channels 1-2)

3. Click outside the dialog to close (settings are saved automatically)

### Windows: ASIO Setup

For best audio performance on Windows, install ASIO drivers:

**If you have a professional audio interface:**
- Download and install the manufacturer's ASIO driver
- Examples: Focusrite, Universal Audio, RME, etc.

**If you're using built-in audio or don't have ASIO drivers:**
- Install [ASIO4ALL](http://www.asio4all.org/) (free, universal ASIO driver)
- After installation, restart VST Runner
- Select "ASIO4ALL" in Audio Settings

### macOS: CoreAudio

CoreAudio works automatically:
- Select your audio interface from the device list
- Configure sample rate and buffer size
- Grant microphone permissions if prompted

### Troubleshooting Audio

**No sound output:**
- Check audio device is selected correctly
- Verify output channels are enabled
- Check output gain is not at zero
- Ensure your audio interface is connected and powered on

**Crackling/Pops:**
- Increase buffer size (512, 1024, or higher)
- Close other audio applications
- Check CPU usage isn't at 100%

**High latency:**
- Decrease buffer size (256, 128, or lower)
- Use ASIO on Windows
- Close unnecessary background applications

## Loading Plugins

### Method 1: Browse for Plugin

1. Click **"Browse..."** button
2. Navigate to your plugin location:

   **Windows common locations:**
   - VST2: `C:\Program Files\VSTPlugins\` or `C:\Program Files\Common Files\VST2\`
   - VST3: `C:\Program Files\Common Files\VST3\`
   - Plugin-specific: `C:\Program Files\Steinberg\VstPlugins\`, etc.

   **macOS common locations:**
   - VST2: `/Library/Audio/Plug-Ins/VST/` or `~/Library/Audio/Plug-Ins/VST/`
   - VST3: `/Library/Audio/Plug-Ins/VST3/` or `~/Library/Audio/Plug-Ins/VST3/`
   - AU: `/Library/Audio/Plug-Ins/Components/`

3. Select the plugin file:
   - **Windows**: `.dll` (VST2) or `.vst3` (VST3 folder)
   - **macOS**: `.vst` (VST2 bundle) or `.vst3` (VST3 bundle)

4. Click **"Load Plugin"**

### Method 2: Manual Path Entry

1. Copy the full path to your plugin file
2. Paste it into the **"Plugin Path"** field
3. Click **"Load Plugin"**

Example paths:
```
Windows:
C:\Program Files\Steinberg\VstPlugins\MyPlugin.dll
C:\Program Files\Common Files\VST3\MyPlugin.vst3

macOS:
/Library/Audio/Plug-Ins/VST/MyPlugin.vst
/Library/Audio/Plug-Ins/VST3/MyPlugin.vst3
```

### After Loading

When a plugin loads successfully:
- The status will show "Loaded: [Plugin Name]"
- The plugin's editor window will open automatically
- The **"Show Plugin Editor"** button will be enabled

## Using the Plugin

### Plugin Editor Window

- The plugin's native GUI opens in a separate window
- Use it just like you would in a DAW
- Window can be closed and reopened with "Show Plugin Editor" button
- Changes are preserved while the plugin is loaded

### Audio/MIDI Input Controls

**Audio Input Enabled:**
- ✓ Checked: Audio from your interface is sent to the plugin
- ☐ Unchecked: Plugin receives silence (useful for MIDI instruments)

**MIDI Input Enabled:**
- ✓ Checked: MIDI from your devices/keyboard is sent to the plugin
- ☐ Unchecked: No MIDI input (useful for audio FX)

**Input Gain:**
- Adjusts the volume of audio going INTO the plugin
- Range: 0.0x to 2.0x (0% to 200%)
- Default: 1.0x (100%, unity gain)
- Use to control input levels or drive the plugin harder

**Output Gain:**
- Adjusts the volume of audio coming OUT of the plugin
- Range: 0.0x to 2.0x (0% to 200%)
- Default: 1.0x (100%, unity gain)
- Use as a master volume control

## Plugin Types

### Audio Effects (FX)

Examples: EQ, Compressor, Reverb, Delay, Distortion

**Setup:**
1. Enable "Audio Input Enabled"
2. Connect an audio source to your interface
3. Set input gain as needed
4. Adjust output gain to taste

**Common use cases:**
- Guitar effects: Connect guitar to input
- Vocal processing: Connect microphone to input
- Mastering: Use with playback from DAW

### MIDI Instruments (VSTi)

Examples: Synthesizers, Samplers, Drum machines

**Setup:**
1. Enable "MIDI Input Enabled"
2. Disable "Audio Input Enabled" (optional, unless needed)
3. Connect a MIDI keyboard/controller
4. Play notes to trigger the instrument
5. Adjust output gain as needed

**MIDI routing:**
- Connect a MIDI keyboard via USB
- Use a MIDI interface
- Use a virtual MIDI device/router

## Tips and Best Practices

### Performance Optimization

1. **Use appropriate buffer size:**
   - Live playing: 128-256 samples
   - Mixing: 512-1024 samples

2. **Close unused plugins:**
   - Unload plugins when not in use
   - Each plugin uses CPU and memory

3. **Monitor CPU usage:**
   - Use Task Manager (Windows) or Activity Monitor (macOS)
   - If CPU is high, increase buffer size

### Working with Different Plugin Formats

**VST2 (.dll / .vst):**
- Older format, widely supported
- Sometimes 32-bit or 64-bit specific
- May require VST2 SDK for building

**VST3 (.vst3):**
- Modern standard, better performance
- Cross-platform compatible
- Recommended format

**AU (macOS only):**
- Apple's Audio Unit format
- Fully supported on macOS
- Located in Components folder

### Saving and Recalling Settings

Currently, plugin state is saved when you:
- Close the application with a plugin loaded
- Load a new plugin (previous state is saved)

**To preserve settings:**
1. Use the plugin's own preset save feature
2. Most plugins have internal preset management
3. Save presets within the plugin editor

## Common Workflows

### Live Guitar Processing

1. Configure audio settings (low latency, ASIO)
2. Connect guitar to audio interface input
3. Load guitar amp/effects plugin
4. Enable audio input
5. Adjust input gain for optimal level
6. Play and adjust plugin parameters
7. Set output gain for desired volume

### MIDI Synthesizer Jamming

1. Connect MIDI keyboard
2. Configure audio settings
3. Load synthesizer plugin
4. Enable MIDI input
5. Disable audio input (optional)
6. Play keyboard
7. Adjust output gain

### Audio Effect Testing/Comparison

1. Load first plugin
2. Configure settings and listen
3. Unload plugin
4. Load second plugin
5. Compare sound and performance

## Keyboard Shortcuts

(Currently not implemented - feature request)

## Known Limitations

- Only one plugin can be loaded at a time
- No plugin chain/routing (use a DAW for this)
- No audio file playback (input is live only)
- No MIDI file playback
- No recording/bouncing capability

## Next Steps

Want more features? VST Runner is designed to be minimal. For full production:

- Use a DAW (Digital Audio Workstation) like:
  - Reaper (affordable, powerful)
  - Ableton Live, FL Studio, Logic Pro (professional)
  - Ardour, LMMS (free, open-source)

VST Runner is perfect for:
- Testing new plugins
- Live performance with a single effect/instrument
- Quick sound design sessions
- Plugin development/testing

## Troubleshooting

### Plugin won't load

1. **Verify plugin path is correct**
2. **Check plugin architecture matches:**
   - VST Runner is 64-bit
   - Plugin must be 64-bit
   - 32-bit plugins won't work
3. **Ensure plugin is valid:**
   - Try plugin in another host (DAW)
   - Reinstall the plugin

### Plugin loads but no sound

1. **Check audio routing:**
   - Audio input enabled (for FX)
   - MIDI input enabled (for instruments)
2. **Verify audio device:**
   - Audio Settings configured correctly
   - Device is active and connected
3. **Check gain settings:**
   - Input gain not at zero
   - Output gain not at zero
4. **Plugin-specific:**
   - Some plugins need initialization
   - Check plugin's internal volume/mix

### Plugin editor doesn't open

1. **Some plugins don't have editors**
   - Check plugin documentation
   - Use parameter automation instead
2. **Editor window might be offscreen:**
   - Close and reopen
   - Check other monitors

### MIDI not working

1. **Enable MIDI input** (toggle button)
2. **Check MIDI device in Audio Settings:**
   - Select your MIDI keyboard/controller
3. **Verify plugin accepts MIDI:**
   - Only MIDI instruments/effects use MIDI
   - Audio FX plugins typically don't need MIDI
4. **Test MIDI device:**
   - Try in another application
   - Check USB connection

## Getting Help

- **Documentation:** Check README.md and BUILD.md
- **Issues:** Report bugs on GitHub
- **JUCE Community:** https://forum.juce.com/
- **Plugin Support:** Contact plugin manufacturer

## Contributing

VST Runner is open source. Contributions welcome for:

- Bug fixes
- Performance improvements
- Additional features (plugin chain, recording, etc.)
- Platform support (Linux)
- Documentation improvements

See the repository for contribution guidelines.
