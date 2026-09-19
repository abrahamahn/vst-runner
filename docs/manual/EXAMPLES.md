# VST Runner Examples

This document provides practical examples for using VST Runner in various scenarios.

## Table of Contents

- [Basic Usage](#basic-usage)
- [Web API Examples](#web-api-examples)
- [Command Line Usage](#command-line-usage)
- [Common Workflows](#common-workflows)

## Basic Usage

### Loading a Plugin (Desktop UI)

1. **Launch VST Runner**
   ```bash
   ./VSTRunner
   ```

2. **Configure Audio**
   - Click "Audio Settings"
   - Select your audio interface
   - Configure sample rate (e.g., 44100 Hz or 48000 Hz)
   - Set buffer size (lower = less latency, higher = more stability)
   - Recommended: 256 or 512 samples for most use cases

3. **Load a Plugin**
   - Click "Browse..." button
   - Navigate to your plugin file:
     - Windows: `C:\Program Files\VSTPlugins\YourPlugin.dll` or `.vst3`
     - macOS: `/Library/Audio/Plug-Ins/VST/YourPlugin.vst` or `/Library/Audio/Plug-Ins/VST3/YourPlugin.vst3`
     - Linux: `/usr/lib/vst/YourPlugin.so`
   - Click "Load Plugin"

4. **Adjust Audio Settings**
   - **Input Gain**: 0.0 - 2.0x (default: 1.0x)
   - **Output Gain**: 0.0 - 2.0x (default: 1.0x)
   - **Audio Input**: Toggle to enable/disable audio passthrough
   - **MIDI Input**: Toggle to enable/disable MIDI input

5. **Use Plugin Editor**
   - Click "Show Plugin Editor" to open the plugin's GUI
   - Adjust plugin parameters
   - Plugin state is preserved while running

### Example: Loading Serum

```
1. Click "Browse..."
2. Navigate to: C:\Program Files\VSTPlugins\Serum_x64.dll
3. Click "Load Plugin"
4. Click "Show Plugin Editor"
5. Adjust oscillators, filters, effects in Serum's GUI
6. Toggle "MIDI Input Enabled" if using MIDI controller
```

## Web API Examples

### Starting the Web Server

**Desktop UI:**
1. Click "Start Web Server"
2. Click "Open Web UI" or navigate to `http://localhost:8080`

**Result:**
```
Web server: http://localhost:8080
```

### Using the Web API

#### Example 1: Get Status

**Request:**
```bash
curl http://localhost:8080/api/status
```

**Response:**
```json
{
  "pluginLoaded": false,
  "pluginName": "",
  "webServerRunning": true
}
```

#### Example 2: Load Plugin via API

**Request:**
```bash
curl -X POST http://localhost:8080/api/plugin/load \
  -H "Content-Type: application/json" \
  -d '{"path": "C:/Program Files/VSTPlugins/Serum_x64.dll"}'
```

**Response:**
```json
{
  "success": true,
  "message": "Plugin loaded successfully"
}
```

#### Example 3: Set Input Gain

**Request:**
```bash
curl -X POST http://localhost:8080/api/audio/input-gain \
  -H "Content-Type: application/json" \
  -d '{"gain": 1.5}'
```

**Response:**
```json
{
  "success": true,
  "gain": 1.5
}
```

#### Example 4: Get Audio Settings

**Request:**
```bash
curl http://localhost:8080/api/audio/settings
```

**Response:**
```json
{
  "inputGain": 1.0,
  "outputGain": 1.0,
  "audioInputEnabled": true,
  "midiInputEnabled": true
}
```

#### Example 5: Scan for Plugins

**Request:**
```bash
curl -X POST http://localhost:8080/api/plugin/scan \
  -H "Content-Type: application/json" \
  -d '{"directory": "C:/Program Files/VSTPlugins"}'
```

**Response:**
```json
{
  "plugins": [
    {
      "name": "Serum",
      "path": "C:/Program Files/VSTPlugins/Serum_x64.dll",
      "format": "VST3"
    },
    {
      "name": "FabFilter Pro-Q 3",
      "path": "C:/Program Files/VSTPlugins/FabFilter Pro-Q 3.vst3",
      "format": "VST3"
    }
  ]
}
```

### Python Example: Automated Plugin Control

```python
import requests
import json

# Base URL
base_url = "http://localhost:8080"

# Load plugin
def load_plugin(plugin_path):
    response = requests.post(
        f"{base_url}/api/plugin/load",
        json={"path": plugin_path}
    )
    return response.json()

# Set gains
def set_gains(input_gain, output_gain):
    requests.post(
        f"{base_url}/api/audio/input-gain",
        json={"gain": input_gain}
    )
    requests.post(
        f"{base_url}/api/audio/output-gain",
        json={"gain": output_gain}
    )

# Example usage
if __name__ == "__main__":
    # Load Serum
    result = load_plugin("C:/Program Files/VSTPlugins/Serum_x64.dll")
    print(f"Plugin loaded: {result}")

    # Set input gain to 1.2x, output to 0.9x
    set_gains(1.2, 0.9)

    # Get current status
    status = requests.get(f"{base_url}/api/status").json()
    print(f"Status: {status}")
```

### JavaScript Example: Web UI Integration

```javascript
// Load plugin
async function loadPlugin(pluginPath) {
    const response = await fetch('http://localhost:8080/api/plugin/load', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ path: pluginPath })
    });
    return await response.json();
}

// Update gain with slider
async function updateInputGain(gain) {
    const response = await fetch('http://localhost:8080/api/audio/input-gain', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ gain: gain })
    });
    return await response.json();
}

// Poll for status updates
async function pollStatus() {
    const response = await fetch('http://localhost:8080/api/status');
    const status = await response.json();

    document.getElementById('plugin-name').textContent =
        status.pluginLoaded ? status.pluginName : 'No plugin loaded';
}

// Update UI every 500ms
setInterval(pollStatus, 500);
```

## Command Line Usage

### Running Tests

```bash
# Build with tests
cmake .. -DBUILD_TESTS=ON
cmake --build .

# Run all tests
./VSTRunnerTests_artefacts/Debug/VSTRunnerTests

# Run specific category
./VSTRunnerTests_artefacts/Debug/VSTRunnerTests --category Core
./VSTRunnerTests_artefacts/Debug/VSTRunnerTests --category Web

# Run specific test
./VSTRunnerTests_artefacts/Debug/VSTRunnerTests --test WebAPIHandler
```

### Running Benchmarks

```bash
# Build with benchmarks
cmake .. -DBUILD_BENCHMARKS=ON
cmake --build .

# Run benchmarks
./VSTRunnerBenchmarks_artefacts/Release/VSTRunnerBenchmarks
```

**Expected Output:**
```
========================================
  VST Runner Performance Benchmarks
========================================

Benchmark: Audio Buffer Processing
-----------------------------------
  Buffer size:   64 | Avg time: 0.0012 ms | Throughput: 106.67 M samples/sec
  Buffer size:  128 | Avg time: 0.0021 ms | Throughput: 121.90 M samples/sec
  Buffer size:  256 | Avg time: 0.0038 ms | Throughput: 134.74 M samples/sec
  Buffer size:  512 | Avg time: 0.0071 ms | Throughput: 144.37 M samples/sec
  ...
```

## Common Workflows

### Workflow 1: Live Performance with MIDI Controller

1. **Setup Audio**
   - Select low-latency audio interface (ASIO on Windows)
   - Set buffer size to 128 or 256 samples
   - Enable audio input if using mic/guitar

2. **Load Instrument Plugin**
   - Browse to synth plugin (Serum, Massive, etc.)
   - Load plugin

3. **Configure MIDI**
   - Ensure "MIDI Input Enabled" is checked
   - Connect MIDI controller
   - Play notes on controller → plugin responds

4. **Adjust Levels**
   - Set input gain based on source (guitar, mic, etc.)
   - Set output gain to prevent clipping
   - Monitor with headphones or speakers

### Workflow 2: Remote Control via Web API

**Use Case:** Control VST Runner from another computer on your network

1. **Start Web Server**
   - Enable web server in VST Runner
   - Note the IP address (e.g., `192.168.1.100:8080`)

2. **Allow Firewall Access**
   - Windows: Allow port 8080 in Windows Firewall
   - macOS: Allow in System Preferences → Security
   - Linux: `sudo ufw allow 8080`

3. **Access from Remote Device**
   ```bash
   # From another computer
   curl http://192.168.1.100:8080/api/status
   ```

4. **Build Custom Controller**
   - Use Python, JavaScript, or any HTTP client
   - Send API requests to control plugins
   - Create custom TouchOSC/MIDI-to-HTTP bridge

### Workflow 3: Automated Plugin Testing

**Use Case:** Test multiple plugins programmatically

```python
import requests
import time

plugins = [
    "C:/Program Files/VSTPlugins/Serum_x64.dll",
    "C:/Program Files/VSTPlugins/Massive.dll",
    "C:/Program Files/VSTPlugins/FabFilter_Pro-Q_3.vst3"
]

for plugin in plugins:
    print(f"Testing {plugin}...")

    # Load plugin
    response = requests.post(
        "http://localhost:8080/api/plugin/load",
        json={"path": plugin}
    )

    if response.json().get("success"):
        print(f"  ✓ Loaded successfully")

        # Wait for plugin to initialize
        time.sleep(2)

        # Unload
        requests.post("http://localhost:8080/api/plugin/unload")
        print(f"  ✓ Unloaded")
    else:
        print(f"  ✗ Failed to load")

    time.sleep(1)
```

### Workflow 4: Docker Container for CI/CD

**Use Case:** Build and test in clean environment

```bash
# Build Docker image
docker build -t vst-runner .

# Run tests in container
docker run --rm vst-runner /vst-runner/build/VSTRunnerTests_artefacts/Release/VSTRunnerTests

# Extract built binary
docker run --rm -v $(pwd)/dist:/dist vst-runner \
  cp /vst-runner/build/VSTRunner_artefacts/Release/VSTRunner /dist/
```

## Troubleshooting Examples

### Example: Plugin Won't Load

**Problem:** "Failed to load plugin" message

**Solution:**
1. Check plugin format matches platform:
   - Windows: `.dll` or `.vst3`
   - macOS: `.vst` or `.vst3`
   - Linux: `.so`

2. Verify plugin path is correct
3. Try loading in another VST host to confirm plugin works
4. Check VST Runner logs for detailed error

### Example: Audio Crackling

**Problem:** Audio has clicks/pops

**Solution:**
1. **Increase Buffer Size**
   - Audio Settings → Buffer Size: 512 or 1024

2. **Check CPU Usage**
   - Close other applications
   - Use lighter plugins

3. **Update Audio Drivers**
   - Windows: Update ASIO drivers
   - macOS: Update system
   - Linux: Update ALSA/JACK

### Example: Web Server Won't Start

**Problem:** "Failed to start web server on port 8080"

**Solution:**
1. **Check if port is in use:**
   ```bash
   # Windows
   netstat -ano | findstr :8080

   # macOS/Linux
   lsof -i :8080
   ```

2. **Kill process using port:**
   ```bash
   # Windows
   taskkill /PID <PID> /F

   # macOS/Linux
   kill -9 <PID>
   ```

3. **Try different port** (requires code modification)

## More Examples

For more examples, see:
- [Web Interface Documentation](WEB_INTERFACE.md)
- [API Reference](WEB_INTERFACE.md#api-endpoints)
- [Architecture Documentation](../dev/ARCHITECTURE.md)

## Contributing Examples

Have a useful workflow or example? Please contribute!

See [CONTRIBUTING.md](../../CONTRIBUTING.md) for guidelines.
