#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "../Core/PluginHostProcessor.h"
#include "AudioSettingsComponent.h"
#include "PluginControlPanel.h"
#include "AudioControlPanel.h"
#include "WebServerPanel.h"
#include "../Web/WebServer.h"

//==============================================================================
/**
    Main application component that provides the UI for the VST Runner.

    Features:
    - Plugin file browser and control
    - Audio settings and gain controls
    - Audio/MIDI I/O toggles
    - Web interface server
*/
class MainComponent : public juce::Component,
                      private juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    void timerCallback() override;

    // Callback handlers (called from panels)
    void browseForPlugin();
    void loadPluginFromPath (const juce::String& path);
    void openAudioSettings();
    void showPluginEditor();
    void closePluginEditor();
    void toggleWebServer();
    void openWebInterface();

    //==============================================================================
    // Audio components
    juce::AudioDeviceManager deviceManager;
    std::unique_ptr<PluginHostProcessor> pluginProcessor;
    juce::AudioProcessorPlayer processorPlayer;

    // Web server
    std::unique_ptr<WebServer> webServer;

    // UI Panels
    juce::Label titleLabel;
    std::unique_ptr<PluginControlPanel> pluginPanel;
    std::unique_ptr<AudioControlPanel> audioPanel;
    std::unique_ptr<WebServerPanel> webServerPanel;
    juce::Label infoLabel;

    // Plugin editor window
    std::unique_ptr<juce::DocumentWindow> pluginEditorWindow;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
