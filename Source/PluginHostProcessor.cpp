#include "PluginHostProcessor.h"

//==============================================================================
PluginHostProcessor::PluginHostProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // Add all available plugin formats
    pluginFormatManager.addDefaultFormats();

    // VST2 and VST3 are added by default via addDefaultFormats()
    // This includes: VST, VST3, AudioUnit (macOS), AAX (if available)
}

PluginHostProcessor::~PluginHostProcessor()
{
    unloadPlugin();
}

//==============================================================================
bool PluginHostProcessor::loadPlugin (const juce::File& pluginPath)
{
    if (!pluginPath.exists())
    {
        juce::AlertWindow::showMessageBoxAsync (
            juce::AlertWindow::WarningIcon,
            "Plugin Not Found",
            "The specified plugin file does not exist:\n" + pluginPath.getFullPathName());
        return false;
    }

    // Unload any existing plugin first
    unloadPlugin();

    // Create a plugin description from the file
    juce::OwnedArray<juce::PluginDescription> typesFound;

    // Try to identify the plugin format and scan it
    for (auto* format : pluginFormatManager.getFormats())
    {
        format->findAllTypesForFile (typesFound, pluginPath.getFullPathName());

        if (typesFound.size() > 0)
            break;
    }

    if (typesFound.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync (
            juce::AlertWindow::WarningIcon,
            "Invalid Plugin",
            "Could not load plugin from:\n" + pluginPath.getFullPathName() +
            "\n\nMake sure it's a valid VST2, VST3, or AU plugin file.");
        return false;
    }

    // Use the first plugin found (VST3 bundles can contain multiple plugins)
    auto* description = typesFound[0];

    // Load the plugin
    juce::String errorMessage;

    loadedPlugin.reset (pluginFormatManager.createPluginInstance (
        *description,
        getSampleRate(),
        getBlockSize(),
        errorMessage).release());

    if (loadedPlugin == nullptr)
    {
        juce::AlertWindow::showMessageBoxAsync (
            juce::AlertWindow::WarningIcon,
            "Plugin Load Failed",
            "Failed to load plugin:\n" + errorMessage);
        return false;
    }

    // Store plugin description
    loadedPluginDescription = loadedPlugin->getName();

    // Prepare the plugin with current audio settings
    if (getSampleRate() > 0)
    {
        loadedPlugin->prepareToPlay (getSampleRate(), getBlockSize());
    }

    // Enable all available buses on the plugin
    loadedPlugin->enableAllBuses();

    DBG ("Successfully loaded plugin: " + loadedPluginDescription);

    return true;
}

void PluginHostProcessor::unloadPlugin()
{
    if (loadedPlugin != nullptr)
    {
        loadedPlugin->releaseResources();
        loadedPlugin.reset();
        loadedPluginDescription.clear();
    }
}

bool PluginHostProcessor::hasPluginLoaded() const
{
    return loadedPlugin != nullptr;
}

juce::String PluginHostProcessor::getLoadedPluginName() const
{
    if (loadedPlugin != nullptr)
        return loadedPlugin->getName();

    return "No Plugin Loaded";
}

//==============================================================================
void PluginHostProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Prepare the loaded plugin if we have one
    if (loadedPlugin != nullptr)
    {
        loadedPlugin->setRateAndBufferSizeDetails (sampleRate, samplesPerBlock);
        loadedPlugin->prepareToPlay (sampleRate, samplesPerBlock);
    }

    // Prepare empty buffer for when no input is needed
    emptyBuffer.setSize (2, samplesPerBlock);
    emptyBuffer.clear();
}

void PluginHostProcessor::releaseResources()
{
    if (loadedPlugin != nullptr)
    {
        loadedPlugin->releaseResources();
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

    // If no plugin is loaded, just pass through or silence
    if (loadedPlugin == nullptr)
    {
        buffer.clear();
        return;
    }

    // Apply input gain
    if (audioInputEnabled)
    {
        buffer.applyGain (inputGain.load());
    }
    else
    {
        // If audio input is disabled, feed silence to the plugin
        buffer.clear();
    }

    // Clear MIDI if MIDI input is disabled
    if (!midiInputEnabled)
    {
        midiMessages.clear();
    }

    // Process the loaded plugin
    loadedPlugin->processBlock (buffer, midiMessages);

    // Apply output gain
    buffer.applyGain (outputGain.load());
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
    if (loadedPlugin != nullptr)
        return loadedPlugin->acceptsMidi();

    return true; // Accept MIDI by default
}

bool PluginHostProcessor::producesMidi() const
{
    if (loadedPlugin != nullptr)
        return loadedPlugin->producesMidi();

    return false;
}

bool PluginHostProcessor::isMidiEffect() const
{
    return false;
}

double PluginHostProcessor::getTailLengthSeconds() const
{
    if (loadedPlugin != nullptr)
        return loadedPlugin->getTailLengthSeconds();

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
    // Save the current plugin state
    if (loadedPlugin != nullptr)
    {
        juce::MemoryBlock pluginState;
        loadedPlugin->getStateInformation (pluginState);

        // Create an XML element to store our state
        juce::XmlElement xml ("VSTRunnerState");
        xml.setAttribute ("pluginState", pluginState.toBase64Encoding());
        xml.setAttribute ("pluginName", loadedPlugin->getName());

        copyXmlToBinary (xml, destData);
    }
}

void PluginHostProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore the plugin state
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName ("VSTRunnerState"))
    {
        auto pluginStateStr = xmlState->getStringAttribute ("pluginState");

        if (loadedPlugin != nullptr && pluginStateStr.isNotEmpty())
        {
            juce::MemoryBlock pluginState;

            if (pluginState.fromBase64Encoding (pluginStateStr))
            {
                loadedPlugin->setStateInformation (pluginState.getData(),
                                                   static_cast<int> (pluginState.getSize()));
            }
        }
    }
}
