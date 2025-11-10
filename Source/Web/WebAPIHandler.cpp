#include "WebAPIHandler.h"

//==============================================================================
WebAPIHandler::WebAPIHandler (PluginHostProcessor* processor, juce::AudioDeviceManager* deviceManager)
    : pluginProcessor (processor), audioDeviceManager (deviceManager)
{
}

WebAPIHandler::~WebAPIHandler()
{
}

//==============================================================================
// API Endpoint Handlers

juce::String WebAPIHandler::handleGetStatus()
{
    juce::var data = createStatusObject();
    return jsonResponse (data);
}

juce::String WebAPIHandler::handleGetPluginInfo()
{
    juce::var data = createPluginInfoObject();
    return jsonResponse (data);
}

juce::String WebAPIHandler::handleLoadPlugin (const juce::String& pluginPath)
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

juce::String WebAPIHandler::handleUnloadPlugin()
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

juce::String WebAPIHandler::handleGetAudioSettings()
{
    juce::var data = createAudioSettingsObject();
    return jsonResponse (data);
}

juce::String WebAPIHandler::handleSetInputGain (float gain)
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

juce::String WebAPIHandler::handleSetOutputGain (float gain)
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

juce::String WebAPIHandler::handleSetAudioInput (bool enabled)
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

juce::String WebAPIHandler::handleSetMidiInput (bool enabled)
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

juce::String WebAPIHandler::handleScanPlugins (const juce::String& directory)
{
    juce::var data (new juce::DynamicObject());
    auto obj = data.getDynamicObject();

    juce::File dir (directory);

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
// Response Helpers

juce::String WebAPIHandler::jsonResponse (const juce::var& data)
{
    juce::String json = juce::JSON::toString (data);

    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: application/json\r\n"
           "Content-Length: " + juce::String (json.length()) + "\r\n"
           "Access-Control-Allow-Origin: *\r\n"
           "\r\n" + json;
}

juce::String WebAPIHandler::errorResponse (const juce::String& message)
{
    juce::var data (new juce::DynamicObject());
    data.getDynamicObject()->setProperty ("success", false);
    data.getDynamicObject()->setProperty ("error", message);

    return jsonResponse (data);
}

//==============================================================================
// Private Helper Methods

juce::var WebAPIHandler::createPluginInfoObject()
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

    return data;
}

juce::var WebAPIHandler::createStatusObject()
{
    juce::var data (new juce::DynamicObject());
    auto obj = data.getDynamicObject();

    obj->setProperty ("running", true);
    obj->setProperty ("hasPlugin", pluginProcessor != nullptr && pluginProcessor->hasPluginLoaded());
    obj->setProperty ("inputGain", pluginProcessor != nullptr ? pluginProcessor->getInputGain() : 1.0f);
    obj->setProperty ("outputGain", pluginProcessor != nullptr ? pluginProcessor->getOutputGain() : 1.0f);
    obj->setProperty ("audioInputEnabled", pluginProcessor != nullptr ? pluginProcessor->isAudioInputEnabled() : true);
    obj->setProperty ("midiInputEnabled", pluginProcessor != nullptr ? pluginProcessor->isMidiInputEnabled() : true);

    return data;
}

juce::var WebAPIHandler::createAudioSettingsObject()
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

    return data;
}
