#pragma once

#include <JuceHeader.h>
#include "../PluginHostProcessor.h"

/**
    Handles all web API business logic for controlling VST Runner.

    Separates API handling from HTTP server concerns.
    Provides clean, testable interface for all plugin and audio operations.
*/
class WebAPIHandler
{
public:
    WebAPIHandler (PluginHostProcessor* processor, juce::AudioDeviceManager* deviceManager);
    ~WebAPIHandler();

    //==============================================================================
    // API Endpoint Handlers

    /** Get current server/plugin status */
    juce::String handleGetStatus();

    /** Get loaded plugin information */
    juce::String handleGetPluginInfo();

    /** Load a VST plugin */
    juce::String handleLoadPlugin (const juce::String& pluginPath);

    /** Unload current plugin */
    juce::String handleUnloadPlugin();

    /** Get audio settings (sample rate, buffer size, etc.) */
    juce::String handleGetAudioSettings();

    /** Set input gain (0.0 to 2.0) */
    juce::String handleSetInputGain (float gain);

    /** Set output gain (0.0 to 2.0) */
    juce::String handleSetOutputGain (float gain);

    /** Enable/disable audio input */
    juce::String handleSetAudioInput (bool enabled);

    /** Enable/disable MIDI input */
    juce::String handleSetMidiInput (bool enabled);

    /** Scan directory for plugins */
    juce::String handleScanPlugins (const juce::String& directory);

    //==============================================================================
    // Response Helpers

    /** Create JSON success response */
    juce::String jsonResponse (const juce::var& data);

    /** Create JSON error response */
    juce::String errorResponse (const juce::String& message);

private:
    //==============================================================================
    // Helper methods for data serialization

    juce::var createPluginInfoObject();
    juce::var createStatusObject();
    juce::var createAudioSettingsObject();

    //==============================================================================
    PluginHostProcessor* pluginProcessor;
    juce::AudioDeviceManager* audioDeviceManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAPIHandler)
};
