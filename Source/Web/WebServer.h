#pragma once

#include <juce_core/juce_core.h>

#include "WebAPIHandler.h"

/**
    HTTP server for web interface.
    Handles HTTP protocol and delegates API logic to WebAPIHandler.
*/
class WebServer : private juce::Thread
{
public:
    WebServer( PluginHostProcessor* processor, juce::AudioDeviceManager* deviceManager );
    ~WebServer() override;

    /** Starts the web server on the specified port. */
    bool start( int port = 8080 );

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

    // HTTP request routing
    juce::String handleHttpRequest( const juce::String& request );
    juce::String handleGetRequest( const juce::String& path );
    juce::String handlePostRequest( const juce::String& path, const juce::String& body );

    // Resource and HTTP response formatting
    juce::String serveFile( const juce::String& filename );
    juce::String
    createHttpResponse( const juce::String& content, const juce::String& contentType, int statusCode = 200 );
    juce::String createHTMLResponse( const juce::String& content, int statusCode = 200 );
    juce::String createJSONResponse( const juce::String& content, int statusCode = 200 );
    juce::String getIndexHTML();
    juce::String getAppJS();
    void loadWebResources();

    std::unique_ptr<WebAPIHandler> apiHandler;

    std::atomic<bool> serverRunning{ false };
    int serverPort = 8080;

    juce::String cachedIndexHTML;
    juce::String cachedAppJS;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( WebServer )
};
