#pragma once

#include <JuceHeader.h>
#include "WebAPIHandler.h"

/**
    HTTP server for web interface.
    Handles HTTP protocol and delegates API logic to WebAPIHandler.

    Refactored in Phase 3 to separate concerns:
    - WebServer: HTTP protocol, resource serving
    - WebAPIHandler: API business logic
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

    // Resource serving
    juce::String serveFile (const juce::String& filename);
    juce::String createHTMLResponse (const juce::String& content, int statusCode = 200);
    juce::String getIndexHTML();
    juce::String getAppJS();
    void loadWebResources();

    // API handler (delegates all business logic)
    std::unique_ptr<WebAPIHandler> apiHandler;

    std::atomic<bool> serverRunning { false };
    int serverPort = 8080;

    // Cached web resources
    juce::String cachedIndexHTML;
    juce::String cachedAppJS;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebServer)
};
