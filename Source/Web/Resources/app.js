// VST Runner Web Interface - JavaScript

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
