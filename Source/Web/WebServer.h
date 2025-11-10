#pragma once

#include <JuceHeader.h>
#include "PluginHostProcessor.h"

/**
    Simple HTTP/WebSocket server for web interface.
    Provides REST API and WebSocket endpoints for controlling VST Runner from a browser.
*/
class WebServer : private juce::Thread
{
public:
    WebServer (PluginHostProcessor* processor, juce::AudioDeviceManager* deviceManager);
    ~WebServer() override;

    /** Starts the web server on the specified port. */
    bool start (int port = 8080);

    /** Stops the web server. */
    void stop();

    /** Returns true if the server is running. */
    bool isRunning() const { return serverRunning; }

    /** Returns the server URL. */
    juce::String getServerURL() const;

    /** Returns the current port. */
    int getPort() const { return serverPort; }

private:
    void run() override;

    // HTTP request handlers
    juce::String handleGetStatus();
    juce::String handleGetPluginInfo();
    juce::String handleLoadPlugin (const juce::String& pluginPath);
    juce::String handleUnloadPlugin();
    juce::String handleGetAudioSettings();
    juce::String handleSetInputGain (float gain);
    juce::String handleSetOutputGain (float gain);
    juce::String handleSetAudioInput (bool enabled);
    juce::String handleSetMidiInput (bool enabled);
    juce::String handleScanPlugins (const juce::String& directory);

    // Helper methods
    juce::String jsonResponse (const juce::var& data);
    juce::String errorResponse (const juce::String& message);
    juce::String serveFile (const juce::String& filename);
    juce::String createHTMLResponse (const juce::String& content, int statusCode = 200);
    juce::String getIndexHTML();
    juce::String getAppJS();
    void loadWebResources();

    PluginHostProcessor* pluginProcessor;
    juce::AudioDeviceManager* audioDeviceManager;

    std::atomic<bool> serverRunning { false };
    int serverPort = 8080;

    // Cached web resources
    juce::String cachedIndexHTML;
    juce::String cachedAppJS;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebServer)
};
