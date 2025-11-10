# Web Interface Guide

VST Runner includes a built-in web interface that allows you to control your VST plugins from any web browser. This guide explains how to use the web interface and its features.

## Overview

The web interface provides a modern, responsive UI for controlling VST Runner without needing to interact with the desktop application. The web server runs locally on your machine, and all audio processing happens natively for optimal performance.

### Architecture

```
┌─────────────┐         HTTP/REST API          ┌──────────────┐
│  Web Browser│ ◄──────────────────────────► │  Web Server  │
│  (Any Device)│         localhost:8080        │  (C++ Thread)│
└─────────────┘                                └──────────────┘
                                                       │
                                                       ▼
                                                ┌──────────────┐
                                                │ VST Plugin   │
                                                │ Host         │
                                                │ (Native)     │
                                                └──────────────┘
                                                       │
                                                       ▼
                                                ┌──────────────┐
                                                │ Audio Device │
                                                │ (ASIO/Core)  │
                                                └──────────────┘
```

**Key Points**:
- Web UI runs in your browser
- HTTP server runs in the desktop application
- VST plugins are loaded and processed natively (not in browser)
- Communication via REST API (polling every 1 second)
- No audio data sent over network - only control messages

## Getting Started

### 1. Start the Web Server

**From Desktop Application**:
1. Launch VST Runner desktop application
2. Click the **"Start Web Server"** button
3. You'll see: "Web server: http://localhost:8080"
4. The button changes to "Stop Web Server"

**Default Settings**:
- Port: 8080
- Host: localhost (127.0.0.1)
- Protocol: HTTP (not HTTPS)

### 2. Access the Web Interface

**Option A: Click "Open Web UI" Button**
- Automatically opens the web interface in your default browser

**Option B: Manual Access**
- Open any web browser
- Navigate to: `http://localhost:8080`

**Option C: From Another Device (Same Network)**
- Find your computer's local IP address (e.g., 192.168.1.100)
- Open browser on another device
- Navigate to: `http://[YOUR_IP]:8080`

### 3. Using the Web Interface

The web interface mirrors the desktop app's functionality with a modern, responsive design.

## Web Interface Features

### Status Section

**Server Status Indicator**:
- 🟢 Green: Connected and running
- 🔴 Red: Disconnected

**Plugin Status**:
- Shows currently loaded plugin name
- Updates in real-time when plugins are loaded/unloaded

### Loading Plugins

**Via Web Interface**:
1. Enter the full path to your plugin file in the "Plugin Path" field
2. Click "Load Plugin"

**Example Paths**:

Windows:
```
C:\Program Files\Steinberg\VstPlugins\MyPlugin.dll
C:\Program Files\Common Files\VST3\MyPlugin.vst3
```

macOS:
```
/Library/Audio/Plug-Ins/VST/MyPlugin.vst
/Library/Audio/Plug-Ins/VST3/MyPlugin.vst3
```

**Important Notes**:
- Path must be absolute (not relative)
- Path must be accessible from the computer running VST Runner
- Plugin editor windows only appear on the desktop (not in web browser)
- Plugin state persists between desktop and web control

### Audio Controls

**Input Gain**:
- Range: 0.0x to 2.0x (0% to 200%)
- Default: 1.0x (unity gain)
- Real-time adjustment with slider
- Affects audio going INTO the plugin

**Output Gain**:
- Range: 0.0x to 2.0x (0% to 200%)
- Default: 1.0x (unity gain)
- Real-time adjustment with slider
- Affects audio coming OUT of the plugin

**Audio Input Toggle**:
- Enable: Plugin receives audio from your interface
- Disable: Plugin receives silence
- Useful for MIDI-only instruments

**MIDI Input Toggle**:
- Enable: Plugin receives MIDI from your devices
- Disable: No MIDI input to plugin
- Useful for audio-only effects

## API Reference

The web interface uses a REST API. You can also access these endpoints programmatically.

### Endpoints

#### GET `/api/status`

Returns current application status.

**Response**:
```json
{
  "running": true,
  "hasPlugin": true,
  "inputGain": 1.0,
  "outputGain": 1.0,
  "audioInputEnabled": true,
  "midiInputEnabled": true
}
```

#### GET `/api/plugin/info`

Returns information about the loaded plugin.

**Response**:
```json
{
  "loaded": true,
  "name": "My Awesome Synth"
}
```

#### POST `/api/plugin/load`

Load a plugin from a file path.

**Request Body**:
```json
{
  "path": "C:\\Program Files\\VSTPlugins\\MyPlugin.dll"
}
```

**Response**:
```json
{
  "success": true,
  "message": "Plugin loaded successfully",
  "pluginName": "My Awesome Synth"
}
```

#### POST `/api/plugin/unload`

Unload the currently loaded plugin.

**Response**:
```json
{
  "success": true,
  "message": "Plugin unloaded"
}
```

#### GET `/api/audio/settings`

Get current audio device settings.

**Response**:
```json
{
  "deviceName": "ASIO4ALL",
  "sampleRate": 48000,
  "bufferSize": 512
}
```

#### POST `/api/control/input-gain`

Set the input gain level.

**Request Body**:
```json
{
  "gain": 1.5
}
```

**Response**:
```json
{
  "success": true,
  "gain": 1.5
}
```

#### POST `/api/control/output-gain`

Set the output gain level.

**Request Body**:
```json
{
  "gain": 0.8
}
```

**Response**:
```json
{
  "success": true,
  "gain": 0.8
}
```

#### POST `/api/control/audio-input`

Enable or disable audio input.

**Request Body**:
```json
{
  "enabled": true
}
```

**Response**:
```json
{
  "success": true,
  "enabled": true
}
```

#### POST `/api/control/midi-input`

Enable or disable MIDI input.

**Request Body**:
```json
{
  "enabled": false
}
```

**Response**:
```json
{
  "success": true,
  "enabled": false
}
```

## Use Cases

### Remote Control from Another Room

1. Start VST Runner on your studio computer
2. Connect audio interface and MIDI keyboard
3. Start the web server
4. Access from tablet/phone while in the recording booth
5. Load and adjust plugins remotely

### Live Performance Setup

1. Pre-load plugin on desktop
2. Access web interface from a secondary device
3. Use web interface for gain staging and quick adjustments
4. Keep desktop app running with plugin editor visible

### Teaching/Demo Sessions

1. Project desktop app to main screen
2. Access web interface from instructor device
3. Control plugins while explaining to students
4. Students can also access to experiment

### Multi-Monitor Workflows

1. Desktop app on primary monitor
2. Web interface in browser on secondary monitor
3. Alternative to juggling multiple plugin windows
4. Cleaner, more organized workspace

## Troubleshooting

### "Failed to start web server on port 8080"

**Cause**: Port 8080 is already in use by another application.

**Solutions**:
1. Close other applications using port 8080
2. Check if another VST Runner instance is running
3. Common culprits: Web servers, development tools, other audio software

**Check what's using port 8080**:

Windows:
```cmd
netstat -ano | findstr :8080
```

macOS/Linux:
```bash
lsof -i :8080
```

### Cannot connect to web interface

**Checks**:
1. Verify web server is started (green indicator in desktop app)
2. Confirm you're using correct URL: `http://localhost:8080`
3. Try `http://127.0.0.1:8080` instead
4. Check firewall isn't blocking the connection
5. Ensure browser isn't using a proxy

### Web interface shows "Disconnected"

**Causes**:
- Desktop application was closed
- Web server was stopped
- Network connectivity issue

**Solutions**:
1. Check desktop app is still running
2. Restart web server in desktop app
3. Refresh the browser page

### Plugin loads on desktop but not from web

**Checks**:
1. Verify you're using the full, absolute path
2. Escape backslashes in Windows paths (or use forward slashes)
3. Ensure path has no typos
4. Plugin file must exist at that location
5. Check plugin is compatible (64-bit VST2/VST3)

### Controls don't update in real-time

**Explanation**:
- Web interface polls every 1 second for updates
- Not a true real-time connection (no WebSocket in current version)
- Slight delay is normal

**Workaround**:
- Use desktop app for immediate feedback
- Web interface best for coarse adjustments

### Cannot access from another device

**Requirements**:
1. Both devices must be on the same local network
2. Firewall must allow incoming connections on port 8080
3. Use the correct IP address of the computer running VST Runner

**Find your IP address**:

Windows:
```cmd
ipconfig
```

Look for "IPv4 Address" under your active network adapter.

macOS:
```bash
ifconfig | grep "inet "
```

Or System Preferences → Network → [Your Connection] → IP Address

**Firewall Settings**:

Windows:
- Windows Defender Firewall → Allow an app
- Add VSTRunner.exe

macOS:
- System Preferences → Security & Privacy → Firewall → Firewall Options
- Add VST Runner to allowed apps

## Security Considerations

### Local Access Only

By default, the web server only accepts connections from localhost (same machine). This is secure for single-user scenarios.

### Network Access

If you enable network access (by connecting from other devices):
- Anyone on your local network can control your VST Runner
- No authentication/password protection
- Not recommended for untrusted networks (public WiFi, etc.)

### Recommended Practices

1. **Use only on trusted networks**: Home, studio
2. **Stop the server when not needed**: Click "Stop Web Server"
3. **Don't expose port 8080 to the internet**: Keep it local
4. **Monitor connections**: Watch for unexpected control changes

### Future Security Enhancements

Planned features for future releases:
- Optional password authentication
- HTTPS support
- Token-based API access
- User access logs

## Performance

### Latency

- **Control latency**: ~1 second (polling interval)
- **Audio latency**: Same as desktop app (no additional latency)
- **Audio processing**: Native (not affected by web interface)

### Resource Usage

- **CPU**: Minimal overhead (~1-2% for web server)
- **Memory**: ~5-10 MB for web server thread
- **Network**: Very low bandwidth (control messages only)

### Optimization Tips

1. Close web browser when not needed
2. Stop web server if not using web interface
3. Web interface doesn't affect audio performance

## Limitations

### Current Limitations

1. **Plugin Editor**: Only appears on desktop, not in web browser
   - Native plugin GUIs require native windowing
   - No remote desktop capability for plugin UIs

2. **Audio Routing**: Cannot change audio settings from web
   - Use desktop app's "Audio Settings" dialog
   - API only provides read-only audio information

3. **Plugin Scanning**: No plugin browser/scanner in web UI
   - Must know exact path to plugin
   - Cannot browse filesystem from web

4. **Real-time Feedback**: 1-second polling delay
   - Not suitable for rapid, precise adjustments
   - Use desktop app for immediate feedback

5. **Single Plugin**: Can only load one plugin at a time
   - Same limitation as desktop app
   - Use a plugin chain host if you need multiple plugins

### Planned Enhancements

Future versions may include:
- WebSocket support for real-time updates
- Plugin preset management
- Plugin scanning/browsing
- Multiple plugin support
- Audio metering/visualization
- Recording capability

## Advanced Usage

### Custom Port

To change the default port (currently hardcoded to 8080), modify:

`Source/MainComponent.cpp` line ~413:
```cpp
if (webServer->start (8080))  // Change 8080 to your desired port
```

Then rebuild the application.

### Programmatic Control

You can control VST Runner from scripts or other applications:

**Python Example**:
```python
import requests

# Load a plugin
response = requests.post('http://localhost:8080/api/plugin/load',
                        json={'path': 'C:\\VST\\MyPlugin.dll'})
print(response.json())

# Set output gain
requests.post('http://localhost:8080/api/control/output-gain',
             json={'gain': 1.5})

# Get status
status = requests.get('http://localhost:8080/api/status').json()
print(f"Plugin loaded: {status['hasPlugin']}")
```

**JavaScript Example** (Node.js):
```javascript
const axios = require('axios');

async function loadPlugin(path) {
  const response = await axios.post('http://localhost:8080/api/plugin/load', {
    path: path
  });
  console.log(response.data);
}

loadPlugin('C:\\VST\\MyPlugin.dll');
```

### Integration with DAWs

While VST Runner is designed as a standalone host, you can use the web API to:
- Automate plugin testing
- Create custom control surfaces
- Build preset management systems
- Integrate with live performance rigs

## Support

For issues or questions about the web interface:
1. Check this documentation
2. Review the main [README.md](README.md) and [SETUP.md](SETUP.md)
3. Check the troubleshooting section above
4. Open an issue on GitHub

## Contributing

Want to improve the web interface? Contributions welcome:
- Enhanced UI/UX
- WebSocket support for real-time updates
- Additional API endpoints
- Authentication/security features
- Mobile app development

See the main repository for contribution guidelines.
