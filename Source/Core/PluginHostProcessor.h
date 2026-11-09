#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>

#include "AudioIOManager.h"
#include "PluginManager.h"

//==============================================================================
/**
    Audio processor that hosts a single VST/VST3 plugin and manages audio/MIDI I/O.

    Responsibilities:
    - Audio processing pipeline
    - Integration with JUCE audio callback
    - Coordination between PluginManager, AudioIOManager, and audio thread
*/
class PluginHostProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    PluginHostProcessor();
    ~PluginHostProcessor() override;

    //==============================================================================
    void prepareToPlay( double sampleRate, int samplesPerBlock ) override;
    void releaseResources() override;

    bool isBusesLayoutSupported( const BusesLayout& layouts ) const override;

    using juce::AudioProcessor::processBlock;
    void processBlock( juce::AudioBuffer<float>&, juce::MidiBuffer& ) override;

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
    void setCurrentProgram( int index ) override;
    const juce::String getProgramName( int index ) override;
    void changeProgramName( int index, const juce::String& newName ) override;

    //==============================================================================
    void getStateInformation( juce::MemoryBlock& destData ) override;
    void setStateInformation( const void* data, int sizeInBytes ) override;

    //==============================================================================
    /** Loads a plugin and prepares it with the host processor's sample rate and block size. */
    bool loadPlugin( const juce::File& pluginPath, juce::String& errorMessage );

    /** Loads a plugin and prepares it with the host processor's sample rate and block size. */
    bool loadPlugin( const juce::File& pluginPath );

    //==============================================================================
    // Access to managers

    PluginManager& getPluginManager() { return pluginManager; }
    const PluginManager& getPluginManager() const { return pluginManager; }

    AudioIOManager& getAudioIOManager() { return audioIOManager; }
    const AudioIOManager& getAudioIOManager() const { return audioIOManager; }

private:
    //==============================================================================
    PluginManager pluginManager;
    AudioIOManager audioIOManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( PluginHostProcessor )
};
