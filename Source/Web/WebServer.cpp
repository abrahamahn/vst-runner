#include "WebServer.h"

#include <BinaryData.h>

//==============================================================================
WebServer::WebServer (PluginHostProcessor* processor, juce::AudioDeviceManager* deviceManager)
    : Thread ("WebServerThread"),
      apiHandler (std::make_unique<WebAPIHandler> (processor, deviceManager))
{
    loadWebResources();
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
    auto serverSocket = std::make_unique<juce::StreamingSocket>();

    if (!serverSocket->createListener (serverPort))
    {
        DBG ("Failed to create server on port " + juce::String (serverPort));
        return;
    }

    serverRunning = true;
    DBG ("Web server started on " + getServerURL());

    while (!threadShouldExit())
    {
        if (serverSocket->waitUntilReady (true, 100) == 1)
        {
            std::unique_ptr<juce::StreamingSocket> clientSocket (serverSocket->waitForNextConnection());

            if (clientSocket != nullptr)
            {
                char buffer[4096];
                const int bytesRead = clientSocket->read (buffer, sizeof (buffer), false);

                if (bytesRead > 0)
                {
                    const juce::String request = juce::String::fromUTF8 (buffer, bytesRead);
                    const juce::String response = handleHttpRequest (request);

                    if (response.isNotEmpty())
                    {
                        clientSocket->write (
                            response.toRawUTF8(),
                            static_cast<int> (response.getNumBytesAsUTF8()));
                    }
                }

                clientSocket->close();
            }
        }
    }

    serverSocket->close();
    serverRunning = false;
}

//==============================================================================
juce::String WebServer::handleHttpRequest (const juce::String& request)
{
    const auto lines = juce::StringArray::fromLines (request);
    if (lines.isEmpty())
        return {};

    const auto tokens = juce::StringArray::fromTokens (lines[0], " ", "");
    if (tokens.size() < 2)
        return {};

    const auto& method = tokens[0];
    const auto& path = tokens[1];

    if (method == "GET")
        return handleGetRequest (path);

    if (method == "POST")
    {
        const auto bodyStart = request.indexOf ("\r\n\r\n");
        const juce::String body = (bodyStart >= 0) ? request.substring (bodyStart + 4) : juce::String();
        return handlePostRequest (path, body);
    }

    return {};
}

juce::String WebServer::handleGetRequest (const juce::String& path)
{
    if (path == "/" || path == "/index.html")
        return serveFile ("index.html");

    if (path == "/app.js")
        return serveFile ("app.js");

    if (path.startsWith ("/api/status"))
        return createJSONResponse (apiHandler->handleGetStatus());

    if (path.startsWith ("/api/plugin/info"))
        return createJSONResponse (apiHandler->handleGetPluginInfo());

    if (path.startsWith ("/api/audio/settings"))
        return createJSONResponse (apiHandler->handleGetAudioSettings());

    return serveFile (path.substring (1));
}

juce::String WebServer::handlePostRequest (const juce::String& path, const juce::String& body)
{
    const auto json = juce::JSON::parse (body);

    if (path.startsWith ("/api/plugin/load"))
        return createJSONResponse (apiHandler->handleLoadPlugin (json["path"].toString()));

    if (path.startsWith ("/api/plugin/unload"))
        return createJSONResponse (apiHandler->handleUnloadPlugin());

    if (path.startsWith ("/api/control/input-gain"))
        return createJSONResponse (apiHandler->handleSetInputGain (static_cast<float> (json["gain"])));

    if (path.startsWith ("/api/control/output-gain"))
        return createJSONResponse (apiHandler->handleSetOutputGain (static_cast<float> (json["gain"])));

    if (path.startsWith ("/api/control/audio-input"))
        return createJSONResponse (apiHandler->handleSetAudioInput (static_cast<bool> (json["enabled"])));

    if (path.startsWith ("/api/control/midi-input"))
        return createJSONResponse (apiHandler->handleSetMidiInput (static_cast<bool> (json["enabled"])));

    if (path.startsWith ("/api/plugins/scan"))
        return createJSONResponse (apiHandler->handleScanPlugins (json["directory"].toString()));

    return createJSONResponse (apiHandler->errorResponse ("Unknown endpoint"), 404);
}

//==============================================================================
juce::String WebServer::serveFile (const juce::String& filename)
{
    if (filename == "index.html" || filename.isEmpty())
        return createHTMLResponse (getIndexHTML());

    if (filename == "app.js")
        return createHttpResponse (getAppJS(), "application/javascript");

    return createHTMLResponse ("<html><body><h1>404 Not Found</h1></body></html>", 404);
}

juce::String WebServer::createHttpResponse (const juce::String& content,
                                            const juce::String& contentType,
                                            int statusCode)
{
    const juce::String statusText = (statusCode == 200) ? "OK" : "Not Found";

    return "HTTP/1.1 " + juce::String (statusCode) + " " + statusText + "\r\n"
           "Content-Type: " + contentType + "\r\n"
           "Content-Length: " + juce::String (content.getNumBytesAsUTF8()) + "\r\n"
           "Access-Control-Allow-Origin: *\r\n"
           "\r\n" + content;
}

juce::String WebServer::createHTMLResponse (const juce::String& content, int statusCode)
{
    return createHttpResponse (content, "text/html", statusCode);
}

juce::String WebServer::createJSONResponse (const juce::String& content, int statusCode)
{
    return createHttpResponse (content, "application/json", statusCode);
}

//==============================================================================
void WebServer::loadWebResources()
{
    const juce::File appDir = juce::File::getSpecialLocation (juce::File::currentApplicationFile)
                                  .getParentDirectory();

    const juce::File htmlFile = appDir.getChildFile ("../../../Source/Web/Resources/index.html");
    const juce::File jsFile = appDir.getChildFile ("../../../Source/Web/Resources/app.js");

    if (htmlFile.existsAsFile())
    {
        cachedIndexHTML = htmlFile.loadFileAsString();
        DBG ("Loaded HTML from external file: " + htmlFile.getFullPathName());
    }

    if (jsFile.existsAsFile())
    {
        cachedAppJS = jsFile.loadFileAsString();
        DBG ("Loaded JavaScript from external file: " + jsFile.getFullPathName());
    }
}

juce::String WebServer::getAppJS()
{
    if (cachedAppJS.isNotEmpty())
        return cachedAppJS;

    return juce::String::fromUTF8 (WebResources::app_js, WebResources::app_jsSize);
}

juce::String WebServer::getIndexHTML()
{
    if (cachedIndexHTML.isNotEmpty())
        return cachedIndexHTML;

    return juce::String::fromUTF8 (WebResources::index_html, WebResources::index_htmlSize);
}
