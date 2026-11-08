#pragma once

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_graphics/juce_graphics.h>

/**
    Manages application configuration and user preferences.

    Handles:
    - Persistent storage of user settings
    - Window size and position
    - Last used plugin paths
    - Audio device settings
    - Web server preferences
    - Gain settings

    Settings are stored in application data folder as XML.
*/
class ConfigurationManager
{
public:
    //==============================================================================
    static ConfigurationManager& getInstance();

    ~ConfigurationManager();

    //==============================================================================
    // Window settings

    void setWindowBounds (const juce::Rectangle<int>& bounds);
    juce::Rectangle<int> getWindowBounds() const;

    //==============================================================================
    // Plugin settings

    void setLastPluginPath (const juce::String& path);
    juce::String getLastPluginPath() const;

    void setLastPluginDirectory (const juce::File& directory);
    juce::File getLastPluginDirectory() const;

    //==============================================================================
    // Audio settings

    void setInputGain (float gain);
    float getInputGain() const;

    void setOutputGain (float gain);
    float getOutputGain() const;

    void setAudioInputEnabled (bool enabled);
    bool isAudioInputEnabled() const;

    void setMidiInputEnabled (bool enabled);
    bool isMidiInputEnabled() const;

    //==============================================================================
    // Web server settings

    void setWebServerPort (int port);
    int getWebServerPort() const;

    void setWebServerAutoStart (bool autoStart);
    bool shouldWebServerAutoStart() const;

    //==============================================================================
    // Persistence

    /** Saves all settings to disk. Called automatically on changes. */
    void save();

    /** Loads settings from disk. Called automatically on first access. */
    void load();

    /** Resets all settings to defaults. */
    void resetToDefaults();

private:
    //==============================================================================
    ConfigurationManager();

    juce::File getSettingsFile() const;
    void loadFromXml (const juce::XmlElement& xml);
    std::unique_ptr<juce::XmlElement> saveToXml() const;

    //==============================================================================
    // Settings data
    juce::Rectangle<int> windowBounds { 100, 100, 700, 600 };
    juce::String lastPluginPath;
    juce::String lastPluginDirectory;

    float inputGain = 1.0f;
    float outputGain = 1.0f;
    bool audioInputEnabled = true;
    bool midiInputEnabled = true;

    int webServerPort = 8080;
    bool webServerAutoStart = false;

    bool settingsLoaded = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConfigurationManager)
};
