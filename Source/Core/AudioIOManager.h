#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_events/juce_events.h>

/**
    Manages audio and MIDI I/O routing and gain controls.

    Responsibilities:
    - Input/output gain management
    - Audio input enable/disable
    - MIDI input enable/disable
    - Signal routing configuration
    - Listener notifications for changes

    This class decouples audio routing logic from the UI and processor.
*/
class AudioIOManager
{
public:
    //==============================================================================
    AudioIOManager();
    ~AudioIOManager();

    //==============================================================================
    // Gain controls

    /** Sets the input gain level (0.0 to 2.0, where 1.0 is unity). */
    void setInputGain (float gain);

    /** Gets the current input gain level. */
    float getInputGain() const { return inputGain.load(); }

    /** Sets the output gain level (0.0 to 2.0, where 1.0 is unity). */
    void setOutputGain (float gain);

    /** Gets the current output gain level. */
    float getOutputGain() const { return outputGain.load(); }

    //==============================================================================
    // Input routing

    /** Enables or disables audio input to the plugin. */
    void setAudioInputEnabled (bool enabled);

    /** Returns whether audio input is enabled. */
    bool isAudioInputEnabled() const { return audioInputEnabled.load(); }

    /** Enables or disables MIDI input to the plugin. */
    void setMidiInputEnabled (bool enabled);

    /** Returns whether MIDI input is enabled. */
    bool isMidiInputEnabled() const { return midiInputEnabled.load(); }

    //==============================================================================
    // Audio processing helpers

    /** Applies input gain to the audio buffer.
        @param buffer   The audio buffer to process
        @param enabled  Whether to apply gain or clear the buffer
    */
    void processInputGain (juce::AudioBuffer<float>& buffer, bool applyGain = true);

    /** Applies output gain to the audio buffer.
        @param buffer   The audio buffer to process
    */
    void processOutputGain (juce::AudioBuffer<float>& buffer);

    /** Filters MIDI messages based on MIDI input enabled state.
        @param midiBuffer   The MIDI buffer to process
    */
    void processMidiInput (juce::MidiBuffer& midiBuffer);

    //==============================================================================
    // Listener interface for I/O changes

    class Listener
    {
    public:
        virtual ~Listener() = default;

        /** Called when input gain changes. */
        virtual void inputGainChanged (float newGain) { juce::ignoreUnused (newGain); }

        /** Called when output gain changes. */
        virtual void outputGainChanged (float newGain) { juce::ignoreUnused (newGain); }

        /** Called when audio input enabled state changes. */
        virtual void audioInputStateChanged (bool enabled) { juce::ignoreUnused (enabled); }

        /** Called when MIDI input enabled state changes. */
        virtual void midiInputStateChanged (bool enabled) { juce::ignoreUnused (enabled); }
    };

    void addListener (Listener* listener);
    void removeListener (Listener* listener);

private:
    //==============================================================================
    void notifyInputGainChanged();
    void notifyOutputGainChanged();
    void notifyAudioInputStateChanged();
    void notifyMidiInputStateChanged();

    //==============================================================================
    std::atomic<float> inputGain { 1.0f };
    std::atomic<float> outputGain { 1.0f };
    std::atomic<bool> audioInputEnabled { true };
    std::atomic<bool> midiInputEnabled { true };

    juce::ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioIOManager)
};
