#pragma once

#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>

/**
    UI panel for audio control operations.

    Handles:
    - Audio settings configuration
    - Input/output gain sliders
    - Audio input enable/disable
    - MIDI input enable/disable
*/
class AudioControlPanel : public juce::Component
{
public:
    //==============================================================================
    AudioControlPanel();
    ~AudioControlPanel() override;

    //==============================================================================
    void paint( juce::Graphics& ) override;
    void resized() override;

    //==============================================================================
    // Callbacks (set by parent component)

    std::function<void()> onOpenAudioSettings;
    std::function<void( float )> onInputGainChanged;
    std::function<void( float )> onOutputGainChanged;
    std::function<void( bool )> onAudioInputToggled;
    std::function<void( bool )> onMidiInputToggled;

    //==============================================================================
    // Update methods (called by parent)

    /** Set input gain slider value */
    void setInputGain( float gain );

    /** Set output gain slider value */
    void setOutputGain( float gain );

    /** Set audio input toggle state */
    void setAudioInputEnabled( bool enabled );

    /** Set MIDI input toggle state */
    void setMidiInputEnabled( bool enabled );

private:
    //==============================================================================
    void audioSettingsButtonClicked();

    //==============================================================================
    // UI Components
    juce::TextButton audioSettingsButton;

    juce::Label inputGainLabel;
    juce::Slider inputGainSlider;

    juce::Label outputGainLabel;
    juce::Slider outputGainSlider;

    juce::ToggleButton audioInputToggle;
    juce::ToggleButton midiInputToggle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( AudioControlPanel )
};
