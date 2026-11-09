#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/**
    Component that displays audio device settings using JUCE's AudioDeviceSelectorComponent.

    This provides a complete UI for:
    - Selecting audio device type (ASIO, WASAPI, CoreAudio, etc.)
    - Choosing input/output devices
    - Configuring sample rate and buffer size
    - Selecting active input/output channels
*/
class AudioSettingsComponent : public juce::Component
{
public:
    //==============================================================================
    AudioSettingsComponent( juce::AudioDeviceManager& deviceManager );
    ~AudioSettingsComponent() override;

    //==============================================================================
    void paint( juce::Graphics& ) override;
    void resized() override;

private:
    //==============================================================================
    juce::AudioDeviceManager& audioDeviceManager;
    std::unique_ptr<juce::AudioDeviceSelectorComponent> audioSetupComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( AudioSettingsComponent )
};
