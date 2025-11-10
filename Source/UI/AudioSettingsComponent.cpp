#include "AudioSettingsComponent.h"

//==============================================================================
AudioSettingsComponent::AudioSettingsComponent (juce::AudioDeviceManager& deviceManager)
    : audioDeviceManager (deviceManager)
{
    // Create the audio device selector with all input/output options
    audioSetupComp.reset (new juce::AudioDeviceSelectorComponent (
        audioDeviceManager,
        0,     // minimum input channels
        256,   // maximum input channels
        0,     // minimum output channels
        256,   // maximum output channels
        true,  // show MIDI input options
        true,  // show MIDI output selector
        true,  // show channels as stereo pairs
        false  // hide advanced options with button
    ));

    addAndMakeVisible (audioSetupComp.get());

    setSize (500, 450);
}

AudioSettingsComponent::~AudioSettingsComponent()
{
}

//==============================================================================
void AudioSettingsComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AudioSettingsComponent::resized()
{
    audioSetupComp->setBounds (getLocalBounds().reduced (10));
}
