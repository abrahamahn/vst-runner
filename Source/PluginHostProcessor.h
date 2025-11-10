#pragma once

#include <JuceHeader.h>
#include "Core/PluginManager.h"
#include "Core/AudioIOManager.h"

//==============================================================================
/**
    Audio processor that hosts a single VST/VST3 plugin and manages audio/MIDI I/O.

    This class has been refactored to use:
    - PluginManager for plugin loading/management
    - AudioIOManager for audio/MIDI routing and gain control

    Responsibilities:
    - Audio processing pipeline
    - Integration with JUCE audio callback
    - Coordination between managers and audio thread
*/
class PluginHostProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    PluginHostProcessor();
    ~PluginHostProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Access to managers

    PluginManager& getPluginManager() { return pluginManager; }
    AudioIOManager& getAudioIOManager() { return audioIOManager; }

    //==============================================================================
    // Legacy compatibility methods (delegate to managers)

    /** @deprecated Use getPluginManager().loadPlugin() instead */
    bool loadPlugin (const juce::File& pluginPath);

    /** @deprecated Use getPluginManager().unloadPlugin() instead */
    void unloadPlugin();

    /** @deprecated Use getPluginManager().hasPluginLoaded() instead */
    bool hasPluginLoaded() const;

    /** @deprecated Use getPluginManager().getLoadedPlugin() instead */
    juce::AudioPluginInstance* getLoadedPlugin() const;

    /** @deprecated Use getPluginManager().getLoadedPluginName() instead */
    juce::String getLoadedPluginName() const;

    /** @deprecated Use getAudioIOManager().setInputGain() instead */
    void setInputGain (float gain);

    /** @deprecated Use getAudioIOManager().setOutputGain() instead */
    void setOutputGain (float gain);

    /** @deprecated Use getAudioIOManager().getInputGain() instead */
    float getInputGain() const;

    /** @deprecated Use getAudioIOManager().getOutputGain() instead */
    float getOutputGain() const;

    /** @deprecated Use getAudioIOManager().setAudioInputEnabled() instead */
    void setAudioInputEnabled (bool enabled);

    /** @deprecated Use getAudioIOManager().isAudioInputEnabled() instead */
    bool isAudioInputEnabled() const;

    /** @deprecated Use getAudioIOManager().setMidiInputEnabled() instead */
    void setMidiInputEnabled (bool enabled);

    /** @deprecated Use getAudioIOManager().isMidiInputEnabled() instead */
    bool isMidiInputEnabled() const;

private:
    //==============================================================================
    // Managers (composition over inheritance)
    PluginManager pluginManager;
    AudioIOManager audioIOManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginHostProcessor)
};
