#include "PluginHostProcessor.h"

//==============================================================================
PluginHostProcessor::PluginHostProcessor()
    : AudioProcessor( BusesProperties()
                          .withInput( "Input", juce::AudioChannelSet::stereo(), true )
                          .withOutput( "Output", juce::AudioChannelSet::stereo(), true ) )
{
}

PluginHostProcessor::~PluginHostProcessor() = default;

//==============================================================================
void PluginHostProcessor::prepareToPlay( double sampleRate, int samplesPerBlock )
{
    if ( auto* plugin = pluginManager.getLoadedPlugin() )
    {
        plugin->setRateAndBufferSizeDetails( sampleRate, samplesPerBlock );
        plugin->prepareToPlay( sampleRate, samplesPerBlock );
    }
}

void PluginHostProcessor::releaseResources()
{
    if ( auto* plugin = pluginManager.getLoadedPlugin() )
    {
        plugin->releaseResources();
    }
}

bool PluginHostProcessor::isBusesLayoutSupported( const BusesLayout& layouts ) const
{
    const auto& mainInput = layouts.getMainInputChannelSet();
    const auto& mainOutput = layouts.getMainOutputChannelSet();

    if ( mainInput != juce::AudioChannelSet::mono() && mainInput != juce::AudioChannelSet::stereo() )
        return false;

    if ( mainOutput != juce::AudioChannelSet::mono() && mainOutput != juce::AudioChannelSet::stereo() )
        return false;

    return mainInput == mainOutput;
}

void PluginHostProcessor::processBlock( juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages )
{
    juce::ScopedNoDenormals noDenormals;

    if ( !pluginManager.hasPluginLoaded() )
    {
        buffer.clear();
        return;
    }

    audioIOManager.processInputGain( buffer, audioIOManager.isAudioInputEnabled() );
    audioIOManager.processMidiInput( midiMessages );

    if ( auto* plugin = pluginManager.getLoadedPlugin() )
    {
        plugin->processBlock( buffer, midiMessages );
    }

    audioIOManager.processOutputGain( buffer );
}

//==============================================================================
juce::AudioProcessorEditor* PluginHostProcessor::createEditor()
{
    return nullptr;
}

bool PluginHostProcessor::hasEditor() const
{
    return false;
}

//==============================================================================
const juce::String PluginHostProcessor::getName() const
{
    return "VST Runner";
}

bool PluginHostProcessor::acceptsMidi() const
{
    if ( auto* plugin = pluginManager.getLoadedPlugin() )
        return plugin->acceptsMidi();

    return true;
}

bool PluginHostProcessor::producesMidi() const
{
    if ( auto* plugin = pluginManager.getLoadedPlugin() )
        return plugin->producesMidi();

    return false;
}

bool PluginHostProcessor::isMidiEffect() const
{
    return false;
}

double PluginHostProcessor::getTailLengthSeconds() const
{
    if ( auto* plugin = pluginManager.getLoadedPlugin() )
        return plugin->getTailLengthSeconds();

    return 0.0;
}

//==============================================================================
int PluginHostProcessor::getNumPrograms()
{
    return 1;
}

int PluginHostProcessor::getCurrentProgram()
{
    return 0;
}

void PluginHostProcessor::setCurrentProgram( int index )
{
    juce::ignoreUnused( index );
}

const juce::String PluginHostProcessor::getProgramName( int index )
{
    juce::ignoreUnused( index );
    return {};
}

void PluginHostProcessor::changeProgramName( int index, const juce::String& newName )
{
    juce::ignoreUnused( index, newName );
}

//==============================================================================
void PluginHostProcessor::getStateInformation( juce::MemoryBlock& destData )
{
    pluginManager.savePluginState( destData );
}

void PluginHostProcessor::setStateInformation( const void* data, int sizeInBytes )
{
    pluginManager.restorePluginState( data, sizeInBytes );
}

//==============================================================================
bool PluginHostProcessor::loadPlugin( const juce::File& pluginPath, juce::String& errorMessage )
{
    auto* plugin = pluginManager.loadPlugin( pluginPath, errorMessage );

    if ( plugin == nullptr )
        return false;

    if ( getSampleRate() > 0.0 )
    {
        plugin->setRateAndBufferSizeDetails( getSampleRate(), getBlockSize() );
        plugin->prepareToPlay( getSampleRate(), getBlockSize() );
    }

    plugin->enableAllBuses();

    return true;
}

bool PluginHostProcessor::loadPlugin( const juce::File& pluginPath )
{
    juce::String errorMessage;
    return loadPlugin( pluginPath, errorMessage );
}
