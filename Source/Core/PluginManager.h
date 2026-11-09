#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

/**
    Centralized manager for all plugin-related operations.

    Responsibilities:
    - Plugin loading and unloading
    - Plugin format management
    - Plugin scanning and discovery
    - Plugin state management
    - Event notifications for plugin changes

    This class separates plugin management logic from audio processing.
*/
class PluginManager
{
public:
    //==============================================================================
    PluginManager();
    ~PluginManager();

    //==============================================================================
    // Plugin loading

    /** Loads a plugin from the specified file path.
        @param pluginPath   The full path to the plugin file
        @param errorMessage Output parameter for error details
        @returns           Pointer to loaded plugin, or nullptr on failure
    */
    juce::AudioPluginInstance* loadPlugin( const juce::File& pluginPath, juce::String& errorMessage );

    /** Loads a plugin from the specified file path without capturing the error string.
        @param pluginPath   The full path to the plugin file
        @returns           true if the plugin was loaded successfully
    */
    bool loadPlugin( const juce::File& pluginPath );

    /** Unloads the currently loaded plugin. */
    void unloadPlugin();

    /** Returns true if a plugin is currently loaded. */
    bool hasPluginLoaded() const;

    /** Returns the loaded plugin instance, or nullptr if no plugin is loaded. */
    juce::AudioPluginInstance* getLoadedPlugin() const { return loadedPlugin.get(); }

    /** Returns the name of the currently loaded plugin, or an empty string if none is loaded. */
    juce::String getLoadedPluginName() const;

    /** Returns the description of the currently loaded plugin. */
    juce::PluginDescription getLoadedPluginDescription() const;

    //==============================================================================
    // Plugin scanning

    /** Scans a directory for plugins.
        @param directory    The directory to scan
        @param recursive    Whether to scan subdirectories
        @returns           Array of plugin descriptions found
    */
    juce::Array<juce::PluginDescription> scanDirectory( const juce::File& directory, bool recursive = true );

    /** Validates whether a file is a valid plugin.
        @param pluginPath   The file to validate
        @returns           true if the file is a valid plugin
    */
    bool isValidPlugin( const juce::File& pluginPath );

    //==============================================================================
    // Plugin state

    /** Saves the current plugin state to a memory block. */
    void savePluginState( juce::MemoryBlock& destData );

    /** Restores plugin state from a memory block. */
    void restorePluginState( const void* data, int sizeInBytes );

    //==============================================================================
    // Listener interface for plugin events

    class Listener
    {
    public:
        virtual ~Listener() = default;

        /** Called when a plugin is successfully loaded. */
        virtual void pluginLoaded( const juce::String& pluginName ) { juce::ignoreUnused( pluginName ); }

        /** Called when a plugin is unloaded. */
        virtual void pluginUnloaded() {}

        /** Called when plugin loading fails. */
        virtual void pluginLoadFailed( const juce::String& errorMessage ) { juce::ignoreUnused( errorMessage ); }
    };

    void addListener( Listener* listener );
    void removeListener( Listener* listener );

private:
    //==============================================================================
    void findPluginDescriptions( const juce::File& pluginPath, juce::OwnedArray<juce::PluginDescription>& typesFound );
    void notifyPluginLoaded( const juce::String& name );
    void notifyPluginUnloaded();
    void notifyPluginLoadFailed( const juce::String& error );

    //==============================================================================
    juce::AudioPluginFormatManager formatManager;
    std::unique_ptr<juce::AudioPluginInstance> loadedPlugin;
    juce::PluginDescription loadedPluginDesc;

    juce::ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( PluginManager )
};
