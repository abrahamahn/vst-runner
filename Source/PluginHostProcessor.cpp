#include "PluginHostProcessor.h"

//==============================================================================
PluginHostProcessor::PluginHostProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // Managers are initialized automatically via member initialization
}

PluginHostProcessor::~PluginHostProcessor()
{
    // Managers are destroyed automatically
}

//==============================================================================
void PluginHostProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Prepare the loaded plugin if we have one
    if (auto* plugin = pluginManager.getLoadedPlugin())
    {
        plugin->setRateAndBufferSizeDetails (sampleRate, samplesPerBlock);
        plugin->prepareToPlay (sampleRate, samplesPerBlock);
    }
}

void PluginHostProcessor::releaseResources()
{
    if (auto* plugin = pluginManager.getLoadedPlugin())
    {
        plugin->releaseResources();
    }
}

bool PluginHostProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Support stereo input/output
    const auto& mainInput  = layouts.getMainInputChannelSet();
    const auto& mainOutput = layouts.getMainOutputChannelSet();

    // We support mono or stereo configurations
    if (mainInput != juce::AudioChannelSet::mono()
        && mainInput != juce::AudioChannelSet::stereo())
        return false;

    if (mainOutput != juce::AudioChannelSet::mono()
        && mainOutput != juce::AudioChannelSet::stereo())
        return false;

    // Input and output layout must match
    return mainInput == mainOutput;
}

void PluginHostProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // If no plugin is loaded, just silence the output
    if (!pluginManager.hasPluginLoaded())
    {
        buffer.clear();
        return;
    }

    // Apply input gain and routing (AudioIOManager handles this)
    audioIOManager.processInputGain (buffer, audioIOManager.isAudioInputEnabled());

    // Process MIDI routing
    audioIOManager.processMidiInput (midiMessages);

    // Process the loaded plugin
    if (auto* plugin = pluginManager.getLoadedPlugin())
    {
        plugin->processBlock (buffer, midiMessages);
    }

    // Apply output gain
    audioIOManager.processOutputGain (buffer);
}

//==============================================================================
juce::AudioProcessorEditor* PluginHostProcessor::createEditor()
{
    // We don't create our own editor - the MainComponent will handle UI
    return nullptr;
}

bool PluginHostProcessor::hasEditor() const
{
    return false;
}

//==============================================================================
const juce::String PluginHostProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginHostProcessor::acceptsMidi() const
{
    if (auto* plugin = pluginManager.getLoadedPlugin())
        return plugin->acceptsMidi();

    return true; // Accept MIDI by default
}

bool PluginHostProcessor::producesMidi() const
{
    if (auto* plugin = pluginManager.getLoadedPlugin())
        return plugin->producesMidi();

    return false;
}

bool PluginHostProcessor::isMidiEffect() const
{
    return false;
}

double PluginHostProcessor::getTailLengthSeconds() const
{
    if (auto* plugin = pluginManager.getLoadedPlugin())
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

void PluginHostProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginHostProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginHostProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginHostProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save the current plugin state using PluginManager
    pluginManager.savePluginState (destData);
}

void PluginHostProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore plugin state using PluginManager
    pluginManager.restorePluginState (data, sizeInBytes);
}

//==============================================================================
// Legacy compatibility methods (delegate to managers)

bool PluginHostProcessor::loadPlugin (const juce::File& pluginPath)
{
    juce::String errorMessage;
    auto* plugin = pluginManager.loadPlugin (pluginPath, errorMessage);

    if (plugin == nullptr)
    {
        juce::AlertWindow::showMessageBoxAsync (
            juce::AlertWindow::WarningIcon,
            "Plugin Load Failed",
            errorMessage);
        return false;
    }

    // Prepare the plugin with current audio settings
    if (getSampleRate() > 0)
    {
        plugin->prepareToPlay (getSampleRate(), getBlockSize());
    }

    // Enable all available buses on the plugin
    plugin->enableAllBuses();

    return true;
}

void PluginHostProcessor::unloadPlugin()
{
    pluginManager.unloadPlugin();
}

bool PluginHostProcessor::hasPluginLoaded() const
{
    return pluginManager.hasPluginLoaded();
}

juce::AudioPluginInstance* PluginHostProcessor::getLoadedPlugin() const
{
    return pluginManager.getLoadedPlugin();
}

juce::String PluginHostProcessor::getLoadedPluginName() const
{
    return pluginManager.getLoadedPluginName();
}

void PluginHostProcessor::setInputGain (float gain)
{
    audioIOManager.setInputGain (gain);
}

void PluginHostProcessor::setOutputGain (float gain)
{
    audioIOManager.setOutputGain (gain);
}

float PluginHostProcessor::getInputGain() const
{
    return audioIOManager.getInputGain();
}

float PluginHostProcessor::getOutputGain() const
{
    return audioIOManager.getOutputGain();
}

void PluginHostProcessor::setAudioInputEnabled (bool enabled)
{
    audioIOManager.setAudioInputEnabled (enabled);
}

bool PluginHostProcessor::isAudioInputEnabled() const
{
    return audioIOManager.isAudioInputEnabled();
}

void PluginHostProcessor::setMidiInputEnabled (bool enabled)
{
    audioIOManager.setMidiInputEnabled (enabled);
}

bool PluginHostProcessor::isMidiInputEnabled() const
{
    return audioIOManager.isMidiInputEnabled();
}
