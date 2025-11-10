#pragma once

#include <JuceHeader.h>
#include "PluginHostProcessor.h"
#include "AudioSettingsComponent.h"

//==============================================================================
/**
    Main application component that provides the UI for the VST Runner.

    Features:
    - Plugin file browser
    - Manual path entry
    - Audio settings dialog
    - Plugin editor window hosting
    - Audio/MIDI I/O controls
    - Input/output volume controls
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

    void browseForPlugin();
    void loadPluginFromPath (const juce::String& path);
    void openAudioSettings();
    void showPluginEditor();
    void closePluginEditor();

    //==============================================================================
    // Audio components
    juce::AudioDeviceManager deviceManager;
    std::unique_ptr<PluginHostProcessor> pluginProcessor;
    juce::AudioProcessorPlayer processorPlayer;

    // UI Components
    juce::Label titleLabel;

    juce::Label pluginPathLabel;
    juce::TextEditor pluginPathEditor;
    juce::TextButton browseButton;
    juce::TextButton loadButton;

    juce::Label statusLabel;

    juce::TextButton audioSettingsButton;
    juce::TextButton showEditorButton;

    juce::Label inputGainLabel;
    juce::Slider inputGainSlider;

    juce::Label outputGainLabel;
    juce::Slider outputGainSlider;

    juce::ToggleButton audioInputToggle;
    juce::ToggleButton midiInputToggle;

    juce::Label infoLabel;

    // Plugin editor window
    std::unique_ptr<juce::DocumentWindow> pluginEditorWindow;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
