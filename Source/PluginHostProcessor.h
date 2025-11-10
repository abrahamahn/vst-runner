#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    Audio processor that hosts a single VST/VST3 plugin and manages audio/MIDI I/O.

    This class handles:
    - Loading and hosting VST2/VST3 plugins
    - Audio input/output routing
    - MIDI input for instrument plugins
    - Real-time audio processing
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
    // Plugin loading and management

    /** Loads a plugin from the specified file path.
        @param pluginPath   The full path to the plugin file (.dll, .vst, .vst3)
        @returns           true if the plugin was loaded successfully
    */
    bool loadPlugin (const juce::File& pluginPath);

    /** Unloads the currently loaded plugin. */
    void unloadPlugin();

    /** Returns true if a plugin is currently loaded. */
    bool hasPluginLoaded() const;

    /** Returns the loaded plugin instance, or nullptr if no plugin is loaded. */
    juce::AudioPluginInstance* getLoadedPlugin() const { return loadedPlugin.get(); }

    /** Returns the name of the currently loaded plugin. */
    juce::String getLoadedPluginName() const;

    //==============================================================================
    // Audio/MIDI control

    /** Sets whether to pass audio input to the plugin. */
    void setAudioInputEnabled (bool enabled) { audioInputEnabled = enabled; }

    /** Returns whether audio input is enabled. */
    bool isAudioInputEnabled() const { return audioInputEnabled; }

    /** Sets whether to pass MIDI input to the plugin. */
    void setMidiInputEnabled (bool enabled) { midiInputEnabled = enabled; }

    /** Returns whether MIDI input is enabled. */
    bool isMidiInputEnabled() const { return midiInputEnabled; }

    /** Sets the input gain (0.0 to 2.0, where 1.0 is unity). */
    void setInputGain (float gain) { inputGain = gain; }

    /** Sets the output gain (0.0 to 2.0, where 1.0 is unity). */
    void setOutputGain (float gain) { outputGain = gain; }

    /** Gets the input gain. */
    float getInputGain() const { return inputGain; }

    /** Gets the output gain. */
    float getOutputGain() const { return outputGain; }

private:
    //==============================================================================
    // Plugin format manager and instance
    juce::AudioPluginFormatManager pluginFormatManager;
    std::unique_ptr<juce::AudioPluginInstance> loadedPlugin;

    // Audio/MIDI routing settings
    bool audioInputEnabled = true;
    bool midiInputEnabled = true;
    std::atomic<float> inputGain { 1.0f };
    std::atomic<float> outputGain { 1.0f };

    // Temporary buffers for audio processing
    juce::AudioBuffer<float> emptyBuffer;

    // Plugin description cache
    juce::String loadedPluginDescription;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginHostProcessor)
};
