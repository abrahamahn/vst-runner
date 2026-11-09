#include "ConfigurationManager.h"

//==============================================================================
ConfigurationManager::ConfigurationManager()
{
    load();
}

ConfigurationManager::~ConfigurationManager()
{
    save();
}

ConfigurationManager& ConfigurationManager::getInstance()
{
    static ConfigurationManager instance;
    return instance;
}

//==============================================================================
// Window settings

void ConfigurationManager::setWindowBounds( const juce::Rectangle<int>& bounds )
{
    windowBounds = bounds;
    save();
}

juce::Rectangle<int> ConfigurationManager::getWindowBounds() const
{
    return windowBounds;
}

//==============================================================================
// Plugin settings

void ConfigurationManager::setLastPluginPath( const juce::String& path )
{
    lastPluginPath = path;

    // Also update the directory
    juce::File file( path );
    if ( file.exists() )
    {
        lastPluginDirectory = file.getParentDirectory().getFullPathName();
    }

    save();
}

juce::String ConfigurationManager::getLastPluginPath() const
{
    return lastPluginPath;
}

void ConfigurationManager::setLastPluginDirectory( const juce::File& directory )
{
    if ( directory.isDirectory() )
    {
        lastPluginDirectory = directory.getFullPathName();
        save();
    }
}

juce::File ConfigurationManager::getLastPluginDirectory() const
{
    if ( lastPluginDirectory.isEmpty() )
    {
        // Default to common plugin directories
#if JUCE_WINDOWS
        return juce::File( "C:\\Program Files\\VSTPlugins" );
#elif JUCE_MAC
        return juce::File( "/Library/Audio/Plug-Ins/VST" );
#else
        return juce::File::getSpecialLocation( juce::File::userHomeDirectory );
#endif
    }

    return juce::File( lastPluginDirectory );
}

//==============================================================================
// Audio settings

void ConfigurationManager::setInputGain( float gain )
{
    inputGain = juce::jlimit( 0.0f, 2.0f, gain );
    save();
}

float ConfigurationManager::getInputGain() const
{
    return inputGain;
}

void ConfigurationManager::setOutputGain( float gain )
{
    outputGain = juce::jlimit( 0.0f, 2.0f, gain );
    save();
}

float ConfigurationManager::getOutputGain() const
{
    return outputGain;
}

void ConfigurationManager::setAudioInputEnabled( bool enabled )
{
    audioInputEnabled = enabled;
    save();
}

bool ConfigurationManager::isAudioInputEnabled() const
{
    return audioInputEnabled;
}

void ConfigurationManager::setMidiInputEnabled( bool enabled )
{
    midiInputEnabled = enabled;
    save();
}

bool ConfigurationManager::isMidiInputEnabled() const
{
    return midiInputEnabled;
}

//==============================================================================
// Web server settings

void ConfigurationManager::setWebServerPort( int port )
{
    webServerPort = juce::jlimit( 1024, 65535, port );
    save();
}

int ConfigurationManager::getWebServerPort() const
{
    return webServerPort;
}

void ConfigurationManager::setWebServerAutoStart( bool autoStart )
{
    webServerAutoStart = autoStart;
    save();
}

bool ConfigurationManager::shouldWebServerAutoStart() const
{
    return webServerAutoStart;
}

//==============================================================================
// Persistence

juce::File ConfigurationManager::getSettingsFile() const
{
    // Store settings in user's application data directory
    juce::File appDataDir = juce::File::getSpecialLocation( juce::File::userApplicationDataDirectory );

    return appDataDir.getChildFile( "VSTRunner" ).getChildFile( "settings.xml" );
}

void ConfigurationManager::save()
{
    auto xml = saveToXml();

    if ( xml != nullptr )
    {
        juce::File settingsFile = getSettingsFile();

        // Create directory if it doesn't exist
        settingsFile.getParentDirectory().createDirectory();

        // Write to file
        if ( !xml->writeTo( settingsFile ) )
        {
            DBG( "Failed to save settings to: " + settingsFile.getFullPathName() );
        }
        else
        {
            DBG( "Settings saved to: " + settingsFile.getFullPathName() );
        }
    }
}

void ConfigurationManager::load()
{
    if ( settingsLoaded )
        return;

    juce::File settingsFile = getSettingsFile();

    if ( settingsFile.existsAsFile() )
    {
        auto xml = juce::XmlDocument::parse( settingsFile );

        if ( xml != nullptr && xml->hasTagName( "VSTRunnerSettings" ) )
        {
            loadFromXml( *xml );
            DBG( "Settings loaded from: " + settingsFile.getFullPathName() );
        }
        else
        {
            DBG( "Failed to parse settings file: " + settingsFile.getFullPathName() );
        }
    }
    else
    {
        DBG( "No settings file found, using defaults" );
    }

    settingsLoaded = true;
}

void ConfigurationManager::resetToDefaults()
{
    windowBounds = juce::Rectangle<int>( 100, 100, 700, 600 );
    lastPluginPath.clear();
    lastPluginDirectory.clear();

    inputGain = 1.0f;
    outputGain = 1.0f;
    audioInputEnabled = true;
    midiInputEnabled = true;

    webServerPort = 8080;
    webServerAutoStart = false;

    save();
}

void ConfigurationManager::loadFromXml( const juce::XmlElement& xml )
{
    // Window settings
    auto* windowElem = xml.getChildByName( "Window" );
    if ( windowElem != nullptr )
    {
        windowBounds.setX( windowElem->getIntAttribute( "x", 100 ) );
        windowBounds.setY( windowElem->getIntAttribute( "y", 100 ) );
        windowBounds.setWidth( windowElem->getIntAttribute( "width", 700 ) );
        windowBounds.setHeight( windowElem->getIntAttribute( "height", 600 ) );
    }

    // Plugin settings
    auto* pluginElem = xml.getChildByName( "Plugin" );
    if ( pluginElem != nullptr )
    {
        lastPluginPath = pluginElem->getStringAttribute( "lastPath" );
        lastPluginDirectory = pluginElem->getStringAttribute( "lastDirectory" );
    }

    // Audio settings
    auto* audioElem = xml.getChildByName( "Audio" );
    if ( audioElem != nullptr )
    {
        inputGain = static_cast<float>( audioElem->getDoubleAttribute( "inputGain", 1.0 ) );
        outputGain = static_cast<float>( audioElem->getDoubleAttribute( "outputGain", 1.0 ) );
        audioInputEnabled = audioElem->getBoolAttribute( "audioInputEnabled", true );
        midiInputEnabled = audioElem->getBoolAttribute( "midiInputEnabled", true );
    }

    // Web server settings
    auto* webElem = xml.getChildByName( "WebServer" );
    if ( webElem != nullptr )
    {
        webServerPort = webElem->getIntAttribute( "port", 8080 );
        webServerAutoStart = webElem->getBoolAttribute( "autoStart", false );
    }
}

std::unique_ptr<juce::XmlElement> ConfigurationManager::saveToXml() const
{
    auto xml = std::make_unique<juce::XmlElement>( "VSTRunnerSettings" );
    xml->setAttribute( "version", "1.0" );

    // Window settings
    auto* windowElem = xml->createNewChildElement( "Window" );
    windowElem->setAttribute( "x", windowBounds.getX() );
    windowElem->setAttribute( "y", windowBounds.getY() );
    windowElem->setAttribute( "width", windowBounds.getWidth() );
    windowElem->setAttribute( "height", windowBounds.getHeight() );

    // Plugin settings
    auto* pluginElem = xml->createNewChildElement( "Plugin" );
    pluginElem->setAttribute( "lastPath", lastPluginPath );
    pluginElem->setAttribute( "lastDirectory", lastPluginDirectory );

    // Audio settings
    auto* audioElem = xml->createNewChildElement( "Audio" );
    audioElem->setAttribute( "inputGain", inputGain );
    audioElem->setAttribute( "outputGain", outputGain );
    audioElem->setAttribute( "audioInputEnabled", audioInputEnabled );
    audioElem->setAttribute( "midiInputEnabled", midiInputEnabled );

    // Web server settings
    auto* webElem = xml->createNewChildElement( "WebServer" );
    webElem->setAttribute( "port", webServerPort );
    webElem->setAttribute( "autoStart", webServerAutoStart );

    return xml;
}
