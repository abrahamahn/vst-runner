#include "PluginManager.h"

//==============================================================================
PluginManager::PluginManager()
{
    // Add all available plugin formats
    formatManager.addDefaultFormats();
}

PluginManager::~PluginManager()
{
    unloadPlugin();
}

//==============================================================================
// Plugin loading

juce::AudioPluginInstance* PluginManager::loadPlugin (const juce::File& pluginPath,
                                                       juce::String& errorMessage)
{
    if (!pluginPath.exists())
    {
        errorMessage = "Plugin file does not exist: " + pluginPath.getFullPathName();
        notifyPluginLoadFailed (errorMessage);
        return nullptr;
    }

    // Unload any existing plugin first
    unloadPlugin();

    // Create a plugin description from the file
    juce::OwnedArray<juce::PluginDescription> typesFound;

    // Try to identify the plugin format and scan it
    for (auto* format : formatManager.getFormats())
    {
        format->findAllTypesForFile (typesFound, pluginPath.getFullPathName());

        if (typesFound.size() > 0)
            break;
    }

    if (typesFound.isEmpty())
    {
        errorMessage = "Could not identify plugin format: " + pluginPath.getFullPathName();
        notifyPluginLoadFailed (errorMessage);
        return nullptr;
    }

    // Use the first plugin found (VST3 bundles can contain multiple plugins)
    loadedPluginDesc = *typesFound[0];

    // Load the plugin
    loadedPlugin.reset (formatManager.createPluginInstance (
        loadedPluginDesc,
        44100.0,  // Default sample rate, will be updated by processor
        512,      // Default block size, will be updated by processor
        errorMessage).release());

    if (loadedPlugin == nullptr)
    {
        notifyPluginLoadFailed (errorMessage);
        return nullptr;
    }

    DBG ("Successfully loaded plugin: " + loadedPlugin->getName());
    notifyPluginLoaded (loadedPlugin->getName());

    return loadedPlugin.get();
}

void PluginManager::unloadPlugin()
{
    if (loadedPlugin != nullptr)
    {
        loadedPlugin.reset();
        loadedPluginDesc = juce::PluginDescription();
        notifyPluginUnloaded();
    }
}

bool PluginManager::hasPluginLoaded() const
{
    return loadedPlugin != nullptr;
}

juce::String PluginManager::getLoadedPluginName() const
{
    if (loadedPlugin != nullptr)
        return loadedPlugin->getName();

    return "No Plugin Loaded";
}

juce::PluginDescription PluginManager::getLoadedPluginDescription() const
{
    return loadedPluginDesc;
}

//==============================================================================
// Plugin scanning

juce::Array<juce::PluginDescription> PluginManager::scanDirectory (const juce::File& directory,
                                                                     bool recursive)
{
    juce::Array<juce::PluginDescription> results;

    if (!directory.isDirectory())
        return results;

    // Get all plugin files in the directory
    auto files = directory.findChildFiles (
        juce::File::findFiles,
        recursive,
        "*.dll;*.vst;*.vst3"
    );

    // Scan each file
    for (const auto& file : files)
    {
        juce::OwnedArray<juce::PluginDescription> typesFound;

        for (auto* format : formatManager.getFormats())
        {
            format->findAllTypesForFile (typesFound, file.getFullPathName());

            if (typesFound.size() > 0)
                break;
        }

        // Add all found plugins to results
        for (auto* desc : typesFound)
        {
            results.add (*desc);
        }
    }

    return results;
}

bool PluginManager::isValidPlugin (const juce::File& pluginPath)
{
    if (!pluginPath.exists())
        return false;

    juce::OwnedArray<juce::PluginDescription> typesFound;

    for (auto* format : formatManager.getFormats())
    {
        format->findAllTypesForFile (typesFound, pluginPath.getFullPathName());

        if (typesFound.size() > 0)
            return true;
    }

    return false;
}

//==============================================================================
// Plugin state

void PluginManager::savePluginState (juce::MemoryBlock& destData)
{
    if (loadedPlugin != nullptr)
    {
        loadedPlugin->getStateInformation (destData);
    }
}

void PluginManager::restorePluginState (const void* data, int sizeInBytes)
{
    if (loadedPlugin != nullptr && data != nullptr && sizeInBytes > 0)
    {
        loadedPlugin->setStateInformation (data, sizeInBytes);
    }
}

//==============================================================================
// Listener management

void PluginManager::addListener (Listener* listener)
{
    listeners.add (listener);
}

void PluginManager::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

void PluginManager::notifyPluginLoaded (const juce::String& name)
{
    listeners.call ([&name] (Listener& l) { l.pluginLoaded (name); });
}

void PluginManager::notifyPluginUnloaded()
{
    listeners.call ([] (Listener& l) { l.pluginUnloaded(); });
}

void PluginManager::notifyPluginLoadFailed (const juce::String& error)
{
    listeners.call ([&error] (Listener& l) { l.pluginLoadFailed (error); });
}
