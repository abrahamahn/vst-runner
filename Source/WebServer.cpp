#include "WebServer.h"

//==============================================================================
WebServer::WebServer (PluginHostProcessor* processor, juce::AudioDeviceManager* deviceManager)
    : Thread ("WebServerThread"),
      pluginProcessor (processor),
      audioDeviceManager (deviceManager)
{
}

WebServer::~WebServer()
{
    stop();
}

//==============================================================================
bool WebServer::start (int port)
{
    if (serverRunning)
        return true;

    serverPort = port;
    startThread();

    // Wait a bit for server to start
    juce::Thread::sleep (100);

    return serverRunning;
}

void WebServer::stop()
{
    if (serverRunning)
    {
        serverRunning = false;
        stopThread (2000);
    }
}

juce::String WebServer::getServerURL() const
{
    return "http://localhost:" + juce::String (serverPort);
}

//==============================================================================
void WebServer::run()
{
    // Create a streaming socket server
    std::unique_ptr<juce::StreamingSocket> serverSocket (new juce::StreamingSocket());

    if (!serverSocket->createListener (serverPort))
    {
        DBG ("Failed to create server on port " + juce::String (serverPort));
        return;
    }

    serverRunning = true;
    DBG ("Web server started on " + getServerURL());

    while (!threadShouldExit())
    {
        // Accept incoming connections with a timeout
        std::unique_ptr<juce::StreamingSocket> clientSocket (new juce::StreamingSocket());

        if (serverSocket->waitForNextConnection (*clientSocket, 100))
        {
            // Read HTTP request
            juce::MemoryBlock requestData;
            char buffer[4096];

            int bytesRead = clientSocket->read (buffer, sizeof (buffer), false);

            if (bytesRead > 0)
            {
                juce::String request (buffer, bytesRead);
                juce::String response;

                // Parse HTTP request
                auto lines = juce::StringArray::fromLines (request);
                if (lines.size() > 0)
                {
                    auto requestLine = lines[0];
                    auto tokens = juce::StringArray::fromTokens (requestLine, " ", "");

                    if (tokens.size() >= 2)
                    {
                        auto method = tokens[0];
                        auto path = tokens[1];

                        // Handle different endpoints
                        if (method == "GET")
                        {
                            if (path == "/" || path == "/index.html")
                            {
                                response = serveFile ("index.html");
                            }
                            else if (path.startsWith ("/api/status"))
                            {
                                response = handleGetStatus();
                            }
                            else if (path.startsWith ("/api/plugin/info"))
                            {
                                response = handleGetPluginInfo();
                            }
                            else if (path.startsWith ("/api/audio/settings"))
                            {
                                response = handleGetAudioSettings();
                            }
                            else
                            {
                                // Serve static files
                                response = serveFile (path.substring (1));
                            }
                        }
                        else if (method == "POST")
                        {
                            // Extract body from request
                            auto bodyStart = request.indexOf ("\r\n\r\n");
                            juce::String body;

                            if (bodyStart >= 0)
                                body = request.substring (bodyStart + 4);

                            // Parse JSON body
                            auto json = juce::JSON::parse (body);

                            if (path.startsWith ("/api/plugin/load"))
                            {
                                auto pluginPath = json["path"].toString();
                                response = handleLoadPlugin (pluginPath);
                            }
                            else if (path.startsWith ("/api/plugin/unload"))
                            {
                                response = handleUnloadPlugin();
                            }
                            else if (path.startsWith ("/api/control/input-gain"))
                            {
                                auto gain = (float) json["gain"];
                                response = handleSetInputGain (gain);
                            }
                            else if (path.startsWith ("/api/control/output-gain"))
                            {
                                auto gain = (float) json["gain"];
                                response = handleSetOutputGain (gain);
                            }
                            else if (path.startsWith ("/api/control/audio-input"))
                            {
                                auto enabled = (bool) json["enabled"];
                                response = handleSetAudioInput (enabled);
                            }
                            else if (path.startsWith ("/api/control/midi-input"))
                            {
                                auto enabled = (bool) json["enabled"];
                                response = handleSetMidiInput (enabled);
                            }
                            else if (path.startsWith ("/api/plugins/scan"))
                            {
                                auto directory = json["directory"].toString();
                                response = handleScanPlugins (directory);
                            }
                        }

                        // Send response
                        if (response.isNotEmpty())
                        {
                            clientSocket->write (response.toRawUTF8(), response.length());
                        }
                    }
                }
            }

            clientSocket->close();
        }
    }

    serverSocket->close();
    serverRunning = false;
}

//==============================================================================
juce::String WebServer::serveFile (const juce::String& filename)
{
    // Serve embedded HTML
    if (filename == "index.html" || filename.isEmpty())
    {
        return createHTMLResponse (getIndexHTML());
    }

    return createHTMLResponse ("<html><body><h1>404 Not Found</h1></body></html>", 404);
}

juce::String WebServer::createHTMLResponse (const juce::String& content, int statusCode)
{
    juce::String statusText = (statusCode == 200) ? "OK" : "Not Found";

    return "HTTP/1.1 " + juce::String (statusCode) + " " + statusText + "\r\n"
           "Content-Type: text/html\r\n"
           "Content-Length: " + juce::String (content.length()) + "\r\n"
           "Access-Control-Allow-Origin: *\r\n"
           "\r\n" + content;
}

//==============================================================================
// HTTP Request Handlers

juce::String WebServer::handleGetStatus()
{
    juce::var data (new juce::DynamicObject());
    auto obj = data.getDynamicObject();

    obj->setProperty ("running", true);
    obj->setProperty ("hasPlugin", pluginProcessor != nullptr && pluginProcessor->hasPluginLoaded());
    obj->setProperty ("inputGain", pluginProcessor != nullptr ? pluginProcessor->getInputGain() : 1.0f);
    obj->setProperty ("outputGain", pluginProcessor != nullptr ? pluginProcessor->getOutputGain() : 1.0f);
    obj->setProperty ("audioInputEnabled", pluginProcessor != nullptr ? pluginProcessor->isAudioInputEnabled() : true);
    obj->setProperty ("midiInputEnabled", pluginProcessor != nullptr ? pluginProcessor->isMidiInputEnabled() : true);

    return jsonResponse (data);
}

juce::String WebServer::handleGetPluginInfo()
{
    juce::var data (new juce::DynamicObject());
    auto obj = data.getDynamicObject();

    if (pluginProcessor != nullptr && pluginProcessor->hasPluginLoaded())
    {
        obj->setProperty ("loaded", true);
        obj->setProperty ("name", pluginProcessor->getLoadedPluginName());
    }
    else
    {
        obj->setProperty ("loaded", false);
        obj->setProperty ("name", "");
    }

    return jsonResponse (data);
}

juce::String WebServer::handleLoadPlugin (const juce::String& pluginPath)
{
    if (pluginProcessor == nullptr)
        return errorResponse ("Plugin processor not available");

    juce::File pluginFile (pluginPath);

    if (pluginProcessor->loadPlugin (pluginFile))
    {
        juce::var data (new juce::DynamicObject());
        data.getDynamicObject()->setProperty ("success", true);
        data.getDynamicObject()->setProperty ("message", "Plugin loaded successfully");
        data.getDynamicObject()->setProperty ("pluginName", pluginProcessor->getLoadedPluginName());
        return jsonResponse (data);
    }

    return errorResponse ("Failed to load plugin");
}

juce::String WebServer::handleUnloadPlugin()
{
    if (pluginProcessor != nullptr)
    {
        pluginProcessor->unloadPlugin();
    }

    juce::var data (new juce::DynamicObject());
    data.getDynamicObject()->setProperty ("success", true);
    data.getDynamicObject()->setProperty ("message", "Plugin unloaded");

    return jsonResponse (data);
}

juce::String WebServer::handleGetAudioSettings()
{
    juce::var data (new juce::DynamicObject());
    auto obj = data.getDynamicObject();

    if (audioDeviceManager != nullptr)
    {
        auto* device = audioDeviceManager->getCurrentAudioDevice();

        if (device != nullptr)
        {
            obj->setProperty ("deviceName", device->getName());
            obj->setProperty ("sampleRate", device->getCurrentSampleRate());
            obj->setProperty ("bufferSize", device->getCurrentBufferSizeSamples());
        }
    }

    return jsonResponse (data);
}

juce::String WebServer::handleSetInputGain (float gain)
{
    if (pluginProcessor != nullptr)
    {
        pluginProcessor->setInputGain (gain);
    }

    juce::var data (new juce::DynamicObject());
    data.getDynamicObject()->setProperty ("success", true);
    data.getDynamicObject()->setProperty ("gain", gain);

    return jsonResponse (data);
}

juce::String WebServer::handleSetOutputGain (float gain)
{
    if (pluginProcessor != nullptr)
    {
        pluginProcessor->setOutputGain (gain);
    }

    juce::var data (new juce::DynamicObject());
    data.getDynamicObject()->setProperty ("success", true);
    data.getDynamicObject()->setProperty ("gain", gain);

    return jsonResponse (data);
}

juce::String WebServer::handleSetAudioInput (bool enabled)
{
    if (pluginProcessor != nullptr)
    {
        pluginProcessor->setAudioInputEnabled (enabled);
    }

    juce::var data (new juce::DynamicObject());
    data.getDynamicObject()->setProperty ("success", true);
    data.getDynamicObject()->setProperty ("enabled", enabled);

    return jsonResponse (data);
}

juce::String WebServer::handleSetMidiInput (bool enabled)
{
    if (pluginProcessor != nullptr)
    {
        pluginProcessor->setMidiInputEnabled (enabled);
    }

    juce::var data (new juce::DynamicObject());
    data.getDynamicObject()->setProperty ("success", true);
    data.getDynamicObject()->setProperty ("enabled", enabled);

    return jsonResponse (data);
}

juce::String WebServer::handleScanPlugins (const juce::String& directory)
{
    juce::var data (new juce::DynamicObject());
    auto obj = data.getDynamicObject();

    juce::File dir (directory);
    juce::Array<juce::File> plugins;

    if (dir.exists() && dir.isDirectory())
    {
        // Scan for plugins
        auto files = dir.findChildFiles (juce::File::findFiles, true, "*.dll;*.vst;*.vst3");

        juce::var pluginArray;

        for (auto& file : files)
        {
            juce::var plugin (new juce::DynamicObject());
            plugin.getDynamicObject()->setProperty ("name", file.getFileNameWithoutExtension());
            plugin.getDynamicObject()->setProperty ("path", file.getFullPathName());
            pluginArray.append (plugin);
        }

        obj->setProperty ("plugins", pluginArray);
        obj->setProperty ("count", files.size());
    }
    else
    {
        obj->setProperty ("plugins", juce::var());
        obj->setProperty ("count", 0);
    }

    return jsonResponse (data);
}

//==============================================================================
// Helper Methods

juce::String WebServer::jsonResponse (const juce::var& data)
{
    juce::String json = juce::JSON::toString (data);

    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: application/json\r\n"
           "Content-Length: " + juce::String (json.length()) + "\r\n"
           "Access-Control-Allow-Origin: *\r\n"
           "\r\n" + json;
}

juce::String WebServer::errorResponse (const juce::String& message)
{
    juce::var data (new juce::DynamicObject());
    data.getDynamicObject()->setProperty ("success", false);
    data.getDynamicObject()->setProperty ("error", message);

    return jsonResponse (data);
}

juce::String WebServer::getIndexHTML()
{
    return R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>VST Runner - Web Interface</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }

        .container {
            max-width: 900px;
            margin: 0 auto;
            background: white;
            border-radius: 12px;
            box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
            overflow: hidden;
        }

        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            text-align: center;
        }

        .header h1 {
            font-size: 32px;
            margin-bottom: 10px;
        }

        .header p {
            opacity: 0.9;
            font-size: 14px;
        }

        .content {
            padding: 30px;
        }

        .section {
            margin-bottom: 30px;
        }

        .section h2 {
            font-size: 20px;
            margin-bottom: 15px;
            color: #333;
            border-bottom: 2px solid #667eea;
            padding-bottom: 8px;
        }

        .status {
            background: #f7fafc;
            padding: 15px;
            border-radius: 8px;
            margin-bottom: 20px;
        }

        .status-item {
            display: flex;
            justify-content: space-between;
            padding: 8px 0;
            border-bottom: 1px solid #e2e8f0;
        }

        .status-item:last-child {
            border-bottom: none;
        }

        .status-label {
            font-weight: 600;
            color: #4a5568;
        }

        .status-value {
            color: #2d3748;
        }

        .status-indicator {
            display: inline-block;
            width: 10px;
            height: 10px;
            border-radius: 50%;
            margin-right: 8px;
        }

        .status-indicator.active {
            background: #48bb78;
        }

        .status-indicator.inactive {
            background: #f56565;
        }

        .input-group {
            margin-bottom: 15px;
        }

        .input-group label {
            display: block;
            margin-bottom: 8px;
            font-weight: 600;
            color: #4a5568;
        }

        .input-group input[type="text"] {
            width: 100%;
            padding: 12px;
            border: 2px solid #e2e8f0;
            border-radius: 6px;
            font-size: 14px;
            transition: border-color 0.3s;
        }

        .input-group input[type="text"]:focus {
            outline: none;
            border-color: #667eea;
        }

        .input-group input[type="range"] {
            width: 100%;
            height: 6px;
            border-radius: 3px;
            background: #e2e8f0;
            outline: none;
            -webkit-appearance: none;
        }

        .input-group input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 20px;
            height: 20px;
            border-radius: 50%;
            background: #667eea;
            cursor: pointer;
        }

        .input-group input[type="range"]::-moz-range-thumb {
            width: 20px;
            height: 20px;
            border-radius: 50%;
            background: #667eea;
            cursor: pointer;
            border: none;
        }

        .slider-value {
            text-align: right;
            color: #667eea;
            font-weight: 600;
            margin-top: 5px;
        }

        .button-group {
            display: flex;
            gap: 10px;
            flex-wrap: wrap;
        }

        button {
            padding: 12px 24px;
            border: none;
            border-radius: 6px;
            font-size: 14px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s;
            flex: 1;
            min-width: 120px;
        }

        button.primary {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }

        button.primary:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);
        }

        button.secondary {
            background: #f7fafc;
            color: #4a5568;
            border: 2px solid #e2e8f0;
        }

        button.secondary:hover {
            background: #e2e8f0;
        }

        button.danger {
            background: #f56565;
            color: white;
        }

        button.danger:hover {
            background: #e53e3e;
        }

        button:disabled {
            opacity: 0.5;
            cursor: not-allowed;
        }

        .toggle {
            display: flex;
            align-items: center;
            padding: 12px;
            background: #f7fafc;
            border-radius: 6px;
            margin-bottom: 10px;
        }

        .toggle label {
            flex: 1;
            font-weight: 600;
            color: #4a5568;
            cursor: pointer;
        }

        .toggle input[type="checkbox"] {
            width: 50px;
            height: 26px;
            position: relative;
            -webkit-appearance: none;
            background: #cbd5e0;
            outline: none;
            border-radius: 13px;
            cursor: pointer;
            transition: 0.3s;
        }

        .toggle input[type="checkbox"]:checked {
            background: #667eea;
        }

        .toggle input[type="checkbox"]:before {
            content: '';
            position: absolute;
            width: 22px;
            height: 22px;
            border-radius: 50%;
            top: 2px;
            left: 2px;
            background: white;
            transition: 0.3s;
        }

        .toggle input[type="checkbox"]:checked:before {
            left: 26px;
        }

        .alert {
            padding: 12px 16px;
            border-radius: 6px;
            margin-bottom: 15px;
            display: none;
        }

        .alert.success {
            background: #c6f6d5;
            color: #22543d;
            border-left: 4px solid #48bb78;
        }

        .alert.error {
            background: #fed7d7;
            color: #742a2a;
            border-left: 4px solid #f56565;
        }

        .alert.show {
            display: block;
            animation: slideIn 0.3s ease;
        }

        @keyframes slideIn {
            from {
                transform: translateY(-10px);
                opacity: 0;
            }
            to {
                transform: translateY(0);
                opacity: 1;
            }
        }

        .footer {
            text-align: center;
            padding: 20px;
            color: #718096;
            font-size: 12px;
            border-top: 1px solid #e2e8f0;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🎵 VST Runner</h1>
            <p>Web Interface - Control your VST plugins from anywhere</p>
        </div>

        <div class="content">
            <div id="alert" class="alert"></div>

            <!-- Status Section -->
            <div class="section">
                <h2>Status</h2>
                <div class="status">
                    <div class="status-item">
                        <span class="status-label">
                            <span id="serverStatus" class="status-indicator"></span>
                            Server Status
                        </span>
                        <span class="status-value" id="serverStatusText">Connecting...</span>
                    </div>
                    <div class="status-item">
                        <span class="status-label">Plugin Loaded</span>
                        <span class="status-value" id="pluginName">None</span>
                    </div>
                </div>
            </div>

            <!-- Plugin Loading Section -->
            <div class="section">
                <h2>Load Plugin</h2>
                <div class="input-group">
                    <label for="pluginPath">Plugin Path</label>
                    <input type="text" id="pluginPath" placeholder="Enter full path to plugin file (e.g., C:\Path\To\Plugin.dll)">
                </div>
                <div class="button-group">
                    <button class="primary" onclick="loadPlugin()">Load Plugin</button>
                    <button class="danger" onclick="unloadPlugin()">Unload Plugin</button>
                </div>
            </div>

            <!-- Audio Controls Section -->
            <div class="section">
                <h2>Audio Controls</h2>

                <div class="input-group">
                    <label for="inputGain">Input Gain</label>
                    <input type="range" id="inputGain" min="0" max="2" step="0.01" value="1" oninput="updateInputGain(this.value)">
                    <div class="slider-value"><span id="inputGainValue">1.00</span>x</div>
                </div>

                <div class="input-group">
                    <label for="outputGain">Output Gain</label>
                    <input type="range" id="outputGain" min="0" max="2" step="0.01" value="1" oninput="updateOutputGain(this.value)">
                    <div class="slider-value"><span id="outputGainValue">1.00</span>x</div>
                </div>

                <div class="toggle">
                    <label for="audioInput">Audio Input Enabled</label>
                    <input type="checkbox" id="audioInput" checked onchange="updateAudioInput(this.checked)">
                </div>

                <div class="toggle">
                    <label for="midiInput">MIDI Input Enabled</label>
                    <input type="checkbox" id="midiInput" checked onchange="updateMidiInput(this.checked)">
                </div>
            </div>
        </div>

        <div class="footer">
            VST Runner Web Interface | Built with JUCE | <span id="serverUrl"></span>
        </div>
    </div>

    <script>
        // Configuration
        const API_BASE = window.location.origin;
        let updateInterval;

        // Initialize
        document.addEventListener('DOMContentLoaded', function() {
            document.getElementById('serverUrl').textContent = API_BASE;
            startStatusUpdates();
        });

        // Start periodic status updates
        function startStatusUpdates() {
            updateStatus();
            updateInterval = setInterval(updateStatus, 1000);
        }

        // Update status from server
        async function updateStatus() {
            try {
                const [statusRes, pluginRes] = await Promise.all([
                    fetch(`${API_BASE}/api/status`),
                    fetch(`${API_BASE}/api/plugin/info`)
                ]);

                const status = await statusRes.json();
                const plugin = await pluginRes.json();

                // Update UI
                document.getElementById('serverStatus').className =
                    'status-indicator ' + (status.running ? 'active' : 'inactive');
                document.getElementById('serverStatusText').textContent =
                    status.running ? 'Running' : 'Disconnected';

                document.getElementById('pluginName').textContent =
                    plugin.loaded ? plugin.name : 'None';

                // Update controls from server state
                if (status.running) {
                    document.getElementById('inputGain').value = status.inputGain;
                    document.getElementById('inputGainValue').textContent = status.inputGain.toFixed(2);

                    document.getElementById('outputGain').value = status.outputGain;
                    document.getElementById('outputGainValue').textContent = status.outputGain.toFixed(2);

                    document.getElementById('audioInput').checked = status.audioInputEnabled;
                    document.getElementById('midiInput').checked = status.midiInputEnabled;
                }
            } catch (error) {
                document.getElementById('serverStatus').className = 'status-indicator inactive';
                document.getElementById('serverStatusText').textContent = 'Disconnected';
            }
        }

        // Load plugin
        async function loadPlugin() {
            const path = document.getElementById('pluginPath').value.trim();

            if (!path) {
                showAlert('Please enter a plugin path', 'error');
                return;
            }

            try {
                const response = await fetch(`${API_BASE}/api/plugin/load`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ path: path })
                });

                const result = await response.json();

                if (result.success) {
                    showAlert(`Plugin loaded: ${result.pluginName}`, 'success');
                    updateStatus();
                } else {
                    showAlert(result.error || 'Failed to load plugin', 'error');
                }
            } catch (error) {
                showAlert('Error loading plugin: ' + error.message, 'error');
            }
        }

        // Unload plugin
        async function unloadPlugin() {
            try {
                const response = await fetch(`${API_BASE}/api/plugin/unload`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' }
                });

                const result = await response.json();

                if (result.success) {
                    showAlert('Plugin unloaded', 'success');
                    document.getElementById('pluginPath').value = '';
                    updateStatus();
                } else {
                    showAlert(result.error || 'Failed to unload plugin', 'error');
                }
            } catch (error) {
                showAlert('Error unloading plugin: ' + error.message, 'error');
            }
        }

        // Update input gain
        async function updateInputGain(value) {
            document.getElementById('inputGainValue').textContent = parseFloat(value).toFixed(2);

            try {
                await fetch(`${API_BASE}/api/control/input-gain`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ gain: parseFloat(value) })
                });
            } catch (error) {
                console.error('Error updating input gain:', error);
            }
        }

        // Update output gain
        async function updateOutputGain(value) {
            document.getElementById('outputGainValue').textContent = parseFloat(value).toFixed(2);

            try {
                await fetch(`${API_BASE}/api/control/output-gain`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ gain: parseFloat(value) })
                });
            } catch (error) {
                console.error('Error updating output gain:', error);
            }
        }

        // Update audio input
        async function updateAudioInput(enabled) {
            try {
                await fetch(`${API_BASE}/api/control/audio-input`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ enabled: enabled })
                });
            } catch (error) {
                console.error('Error updating audio input:', error);
            }
        }

        // Update MIDI input
        async function updateMidiInput(enabled) {
            try {
                await fetch(`${API_BASE}/api/control/midi-input`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ enabled: enabled })
                });
            } catch (error) {
                console.error('Error updating MIDI input:', error);
            }
        }

        // Show alert message
        function showAlert(message, type) {
            const alert = document.getElementById('alert');
            alert.textContent = message;
            alert.className = `alert ${type} show`;

            setTimeout(() => {
                alert.className = 'alert';
            }, 5000);
        }
    </script>
</body>
</html>
)HTML";
}
