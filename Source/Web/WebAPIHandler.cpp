#include "WebAPIHandler.h"

//==============================================================================
WebAPIHandler::WebAPIHandler( PluginHostProcessor* processor, juce::AudioDeviceManager* deviceManager )
    : pluginProcessor( processor ), audioDeviceManager( deviceManager )
{
}

WebAPIHandler::~WebAPIHandler() = default;

//==============================================================================
// API Endpoint Handlers

juce::String WebAPIHandler::handleGetStatus()
{
    return jsonResponse( createStatusObject() );
}

juce::String WebAPIHandler::handleGetPluginInfo()
{
    return jsonResponse( createPluginInfoObject() );
}

juce::String WebAPIHandler::handleLoadPlugin( const juce::String& pluginPath )
{
    if ( pluginProcessor == nullptr )
        return errorResponse( "Plugin processor not available" );

    juce::File pluginFile( pluginPath );
    juce::String errorMessage;

    if ( pluginProcessor->loadPlugin( pluginFile, errorMessage ) )
    {
        juce::var data( new juce::DynamicObject() );
        auto* obj = data.getDynamicObject();
        obj->setProperty( "success", true );
        obj->setProperty( "message", "Plugin loaded successfully" );
        obj->setProperty( "pluginName", pluginProcessor->getPluginManager().getLoadedPluginName() );
        return jsonResponse( data );
    }

    return errorResponse( errorMessage.isNotEmpty() ? errorMessage : "Failed to load plugin" );
}

juce::String WebAPIHandler::handleUnloadPlugin()
{
    if ( pluginProcessor != nullptr )
    {
        pluginProcessor->getPluginManager().unloadPlugin();
    }

    juce::var data( new juce::DynamicObject() );
    auto* obj = data.getDynamicObject();
    obj->setProperty( "success", true );
    obj->setProperty( "message", "Plugin unloaded" );

    return jsonResponse( data );
}

juce::String WebAPIHandler::handleGetAudioSettings()
{
    return jsonResponse( createAudioSettingsObject() );
}

juce::String WebAPIHandler::handleSetInputGain( float gain )
{
    if ( pluginProcessor != nullptr )
    {
        pluginProcessor->getAudioIOManager().setInputGain( gain );
    }

    juce::var data( new juce::DynamicObject() );
    auto* obj = data.getDynamicObject();
    obj->setProperty( "success", true );
    obj->setProperty( "gain", pluginProcessor != nullptr ? pluginProcessor->getAudioIOManager().getInputGain() : gain );

    return jsonResponse( data );
}

juce::String WebAPIHandler::handleSetOutputGain( float gain )
{
    if ( pluginProcessor != nullptr )
    {
        pluginProcessor->getAudioIOManager().setOutputGain( gain );
    }

    juce::var data( new juce::DynamicObject() );
    auto* obj = data.getDynamicObject();
    obj->setProperty( "success", true );
    obj->setProperty(
        "gain", pluginProcessor != nullptr ? pluginProcessor->getAudioIOManager().getOutputGain() : gain );

    return jsonResponse( data );
}

juce::String WebAPIHandler::handleSetAudioInput( bool enabled )
{
    if ( pluginProcessor != nullptr )
    {
        pluginProcessor->getAudioIOManager().setAudioInputEnabled( enabled );
    }

    juce::var data( new juce::DynamicObject() );
    auto* obj = data.getDynamicObject();
    obj->setProperty( "success", true );
    obj->setProperty( "enabled", enabled );

    return jsonResponse( data );
}

juce::String WebAPIHandler::handleSetMidiInput( bool enabled )
{
    if ( pluginProcessor != nullptr )
    {
        pluginProcessor->getAudioIOManager().setMidiInputEnabled( enabled );
    }

    juce::var data( new juce::DynamicObject() );
    auto* obj = data.getDynamicObject();
    obj->setProperty( "success", true );
    obj->setProperty( "enabled", enabled );

    return jsonResponse( data );
}

juce::String WebAPIHandler::handleScanPlugins( const juce::String& directory )
{
    juce::var data( new juce::DynamicObject() );
    auto* obj = data.getDynamicObject();
    juce::Array<juce::var> pluginArray;

    juce::File dir( directory );

    if ( dir.exists() && dir.isDirectory() )
    {
        auto files = dir.findChildFiles( juce::File::findFiles, true, "*.dll;*.vst;*.vst3" );

        for ( const auto& file : files )
        {
            juce::var plugin( new juce::DynamicObject() );
            auto* pluginObj = plugin.getDynamicObject();
            pluginObj->setProperty( "name", file.getFileNameWithoutExtension() );
            pluginObj->setProperty( "path", file.getFullPathName() );
            pluginArray.add( plugin );
        }
    }

    obj->setProperty( "plugins", pluginArray );
    obj->setProperty( "count", pluginArray.size() );

    return jsonResponse( data );
}

//==============================================================================
// Response Helpers

juce::String WebAPIHandler::jsonResponse( const juce::var& data )
{
    return juce::JSON::toString( data, true );
}

juce::String WebAPIHandler::errorResponse( const juce::String& message )
{
    juce::var data( new juce::DynamicObject() );
    auto* obj = data.getDynamicObject();
    obj->setProperty( "success", false );
    obj->setProperty( "error", message );

    return jsonResponse( data );
}

//==============================================================================
// Private Helper Methods

juce::var WebAPIHandler::createPluginInfoObject()
{
    juce::var data( new juce::DynamicObject() );
    auto* obj = data.getDynamicObject();

    const bool loaded = pluginProcessor != nullptr && pluginProcessor->getPluginManager().hasPluginLoaded();
    obj->setProperty( "loaded", loaded );
    obj->setProperty( "name", loaded ? pluginProcessor->getPluginManager().getLoadedPluginName() : juce::String() );

    return data;
}

juce::var WebAPIHandler::createStatusObject()
{
    juce::var data( new juce::DynamicObject() );
    auto* obj = data.getDynamicObject();

    const bool hasPlugin = pluginProcessor != nullptr && pluginProcessor->getPluginManager().hasPluginLoaded();
    const juce::String pluginName =
        hasPlugin ? pluginProcessor->getPluginManager().getLoadedPluginName() : juce::String();

    obj->setProperty( "running", true );
    obj->setProperty( "hasPlugin", hasPlugin );
    obj->setProperty( "pluginLoaded", hasPlugin );
    obj->setProperty( "pluginName", pluginName );
    obj->setProperty(
        "inputGain", pluginProcessor != nullptr ? pluginProcessor->getAudioIOManager().getInputGain() : 1.0f );
    obj->setProperty(
        "outputGain", pluginProcessor != nullptr ? pluginProcessor->getAudioIOManager().getOutputGain() : 1.0f );
    obj->setProperty(
        "audioInputEnabled",
        pluginProcessor != nullptr ? pluginProcessor->getAudioIOManager().isAudioInputEnabled() : true );
    obj->setProperty(
        "midiInputEnabled",
        pluginProcessor != nullptr ? pluginProcessor->getAudioIOManager().isMidiInputEnabled() : true );

    return data;
}

juce::var WebAPIHandler::createAudioSettingsObject()
{
    juce::var data( new juce::DynamicObject() );
    auto* obj = data.getDynamicObject();

    obj->setProperty(
        "inputGain", pluginProcessor != nullptr ? pluginProcessor->getAudioIOManager().getInputGain() : 1.0f );
    obj->setProperty(
        "outputGain", pluginProcessor != nullptr ? pluginProcessor->getAudioIOManager().getOutputGain() : 1.0f );
    obj->setProperty(
        "audioInputEnabled",
        pluginProcessor != nullptr ? pluginProcessor->getAudioIOManager().isAudioInputEnabled() : true );
    obj->setProperty(
        "midiInputEnabled",
        pluginProcessor != nullptr ? pluginProcessor->getAudioIOManager().isMidiInputEnabled() : true );

    if ( audioDeviceManager != nullptr )
    {
        if ( auto* device = audioDeviceManager->getCurrentAudioDevice() )
        {
            obj->setProperty( "deviceName", device->getName() );
            obj->setProperty( "sampleRate", device->getCurrentSampleRate() );
            obj->setProperty( "bufferSize", device->getCurrentBufferSizeSamples() );
        }
    }

    return data;
}
