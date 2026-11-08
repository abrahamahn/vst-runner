#include "AudioIOManager.h"

//==============================================================================
AudioIOManager::AudioIOManager()
{
}

AudioIOManager::~AudioIOManager()
{
}

//==============================================================================
// Gain controls

void AudioIOManager::setInputGain (float gain)
{
    float clampedGain = juce::jlimit (0.0f, 2.0f, gain);

    if (!juce::approximatelyEqual (inputGain.load(), clampedGain))
    {
        inputGain.store (clampedGain);
        notifyInputGainChanged();
    }
}

void AudioIOManager::setOutputGain (float gain)
{
    float clampedGain = juce::jlimit (0.0f, 2.0f, gain);

    if (!juce::approximatelyEqual (outputGain.load(), clampedGain))
    {
        outputGain.store (clampedGain);
        notifyOutputGainChanged();
    }
}

//==============================================================================
// Input routing

void AudioIOManager::setAudioInputEnabled (bool enabled)
{
    if (audioInputEnabled.load() != enabled)
    {
        audioInputEnabled.store (enabled);
        notifyAudioInputStateChanged();
    }
}

void AudioIOManager::setMidiInputEnabled (bool enabled)
{
    if (midiInputEnabled.load() != enabled)
    {
        midiInputEnabled.store (enabled);
        notifyMidiInputStateChanged();
    }
}

//==============================================================================
// Audio processing helpers

void AudioIOManager::processInputGain (juce::AudioBuffer<float>& buffer, bool applyGain)
{
    if (!audioInputEnabled.load() || !applyGain)
    {
        // If audio input is disabled, clear the buffer (feed silence to plugin)
        buffer.clear();
    }
    else
    {
        // Apply input gain
        float gain = inputGain.load();

        if (!juce::approximatelyEqual (gain, 1.0f))
        {
            buffer.applyGain (gain);
        }
    }
}

void AudioIOManager::processOutputGain (juce::AudioBuffer<float>& buffer)
{
    float gain = outputGain.load();

    if (!juce::approximatelyEqual (gain, 1.0f))
    {
        buffer.applyGain (gain);
    }
}

void AudioIOManager::processMidiInput (juce::MidiBuffer& midiBuffer)
{
    if (!midiInputEnabled.load())
    {
        // If MIDI input is disabled, clear the buffer
        midiBuffer.clear();
    }
}

//==============================================================================
// Listener management

void AudioIOManager::addListener (Listener* listener)
{
    listeners.add (listener);
}

void AudioIOManager::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

void AudioIOManager::notifyInputGainChanged()
{
    float gain = inputGain.load();
    listeners.call ([gain] (Listener& l) { l.inputGainChanged (gain); });
}

void AudioIOManager::notifyOutputGainChanged()
{
    float gain = outputGain.load();
    listeners.call ([gain] (Listener& l) { l.outputGainChanged (gain); });
}

void AudioIOManager::notifyAudioInputStateChanged()
{
    bool enabled = audioInputEnabled.load();
    listeners.call ([enabled] (Listener& l) { l.audioInputStateChanged (enabled); });
}

void AudioIOManager::notifyMidiInputStateChanged()
{
    bool enabled = midiInputEnabled.load();
    listeners.call ([enabled] (Listener& l) { l.midiInputStateChanged (enabled); });
}
