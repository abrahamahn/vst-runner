#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    pluginProcessor = std::make_unique<PluginHostProcessor>();

    deviceManager.initialiseWithDefaultDevices( 2, 2 );

    processorPlayer.setProcessor( pluginProcessor.get() );
    deviceManager.addAudioCallback( &processorPlayer );

    titleLabel.setText( "VST Runner", juce::dontSendNotification );
    titleLabel.setFont( juce::Font( juce::FontOptions( 24.0f ).withStyle( "Bold" ) ) );
    titleLabel.setJustificationType( juce::Justification::centred );
    addAndMakeVisible( titleLabel );

    pluginPanel = std::make_unique<PluginControlPanel>();
    pluginPanel->onBrowseForPlugin = [this] { browseForPlugin(); };
    pluginPanel->onLoadPlugin = [this]( const juce::String& path ) { loadPluginFromPath( path ); };
    pluginPanel->onShowPluginEditor = [this] { showPluginEditor(); };
    addAndMakeVisible( pluginPanel.get() );

    audioPanel = std::make_unique<AudioControlPanel>();
    audioPanel->onOpenAudioSettings = [this] { openAudioSettings(); };
    audioPanel->onInputGainChanged = [this]( float gain )
    {
        if ( pluginProcessor != nullptr )
            pluginProcessor->getAudioIOManager().setInputGain( gain );
    };
    audioPanel->onOutputGainChanged = [this]( float gain )
    {
        if ( pluginProcessor != nullptr )
            pluginProcessor->getAudioIOManager().setOutputGain( gain );
    };
    audioPanel->onAudioInputToggled = [this]( bool enabled )
    {
        if ( pluginProcessor != nullptr )
            pluginProcessor->getAudioIOManager().setAudioInputEnabled( enabled );
    };
    audioPanel->onMidiInputToggled = [this]( bool enabled )
    {
        if ( pluginProcessor != nullptr )
            pluginProcessor->getAudioIOManager().setMidiInputEnabled( enabled );
    };
    addAndMakeVisible( audioPanel.get() );

    webServerPanel = std::make_unique<WebServerPanel>();
    webServerPanel->onToggleWebServer = [this] { toggleWebServer(); };
    webServerPanel->onOpenWebUI = [this] { openWebInterface(); };
    addAndMakeVisible( webServerPanel.get() );

    infoLabel.setText( "Load a VST2 (.dll/.vst) or VST3 (.vst3) plugin to begin", juce::dontSendNotification );
    infoLabel.setJustificationType( juce::Justification::centred );
    infoLabel.setFont( juce::Font( juce::FontOptions( 12.0f ).withStyle( "Italic" ) ) );
    infoLabel.setColour( juce::Label::textColourId, juce::Colours::grey );
    addAndMakeVisible( infoLabel );

    webServer = std::make_unique<WebServer>( pluginProcessor.get(), &deviceManager );

    startTimer( 100 );

    setSize( 700, 600 );
}

MainComponent::~MainComponent()
{
    stopTimer();

    if ( webServer != nullptr )
    {
        webServer->stop();
        webServer.reset();
    }

    closePluginEditor();

    deviceManager.removeAudioCallback( &processorPlayer );
    processorPlayer.setProcessor( nullptr );

    pluginProcessor = nullptr;
}

//==============================================================================
void MainComponent::paint( juce::Graphics& g )
{
    g.fillAll( getLookAndFeel().findColour( juce::ResizableWindow::backgroundColourId ) );

    g.setColour( juce::Colours::grey );
    g.drawRect( getLocalBounds().reduced( 10 ), 1 );
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced( 20 );

    titleLabel.setBounds( area.removeFromTop( 40 ) );
    area.removeFromTop( 10 );

    pluginPanel->setBounds( area.removeFromTop( 200 ) );
    area.removeFromTop( 10 );

    audioPanel->setBounds( area.removeFromTop( 180 ) );
    area.removeFromTop( 10 );

    webServerPanel->setBounds( area.removeFromTop( 120 ) );
    area.removeFromTop( 15 );

    infoLabel.setBounds( area.removeFromTop( 40 ) );
}

//==============================================================================
void MainComponent::timerCallback()
{
    if ( pluginProcessor != nullptr && pluginProcessor->getPluginManager().hasPluginLoaded() )
    {
        pluginPanel->setStatusText( "Loaded: " + pluginProcessor->getPluginManager().getLoadedPluginName() );
        pluginPanel->setEditorButtonEnabled( true );
    }
    else
    {
        pluginPanel->setStatusText( "No plugin loaded" );
        pluginPanel->setEditorButtonEnabled( false );
    }
}

void MainComponent::browseForPlugin()
{
    juce::FileChooser chooser(
        "Select a VST Plugin", juce::File::getSpecialLocation( juce::File::userHomeDirectory ),
#if JUCE_WINDOWS
        "*.dll;*.vst3"
#elif JUCE_MAC
        "*.vst;*.vst3"
#else
        "*"
#endif
    );

    if ( chooser.browseForFileToOpen() )
    {
        auto file = chooser.getResult();
        pluginPanel->setPluginPath( file.getFullPathName() );
    }
}

void MainComponent::loadPluginFromPath( const juce::String& path )
{
    if ( path.isEmpty() )
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon, "No Path Specified",
            "Please enter a plugin path or use the Browse button." );
        return;
    }

    juce::File pluginFile( path );

    closePluginEditor();

    juce::String errorMessage;
    if ( pluginProcessor != nullptr && pluginProcessor->loadPlugin( pluginFile, errorMessage ) )
    {
        infoLabel.setText(
            "Plugin loaded successfully! Click 'Show Plugin Editor' to open it.", juce::dontSendNotification );
        infoLabel.setColour( juce::Label::textColourId, juce::Colours::lightgreen );

        showPluginEditor();
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon, "Plugin Load Failed",
            errorMessage.isNotEmpty() ? errorMessage : "Failed to load plugin." );

        infoLabel.setText( "Failed to load plugin. Check the path and plugin format.", juce::dontSendNotification );
        infoLabel.setColour( juce::Label::textColourId, juce::Colours::red );
    }
}

void MainComponent::openAudioSettings()
{
    juce::DialogWindow::LaunchOptions options;

    auto* audioSettings = new AudioSettingsComponent( deviceManager );

    options.content.setOwned( audioSettings );
    options.dialogTitle = "Audio Settings";
    options.dialogBackgroundColour = getLookAndFeel().findColour( juce::ResizableWindow::backgroundColourId );
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;

    options.launchAsync();
}

void MainComponent::showPluginEditor()
{
    if ( pluginProcessor == nullptr || !pluginProcessor->getPluginManager().hasPluginLoaded() )
        return;

    auto* plugin = pluginProcessor->getPluginManager().getLoadedPlugin();

    if ( plugin == nullptr )
        return;

    closePluginEditor();

    if ( plugin->hasEditor() )
    {
        auto* editor = plugin->createEditorIfNeeded();

        if ( editor != nullptr )
        {
            pluginEditorWindow = std::make_unique<juce::DocumentWindow>(
                plugin->getName(), getLookAndFeel().findColour( juce::ResizableWindow::backgroundColourId ),
                juce::DocumentWindow::closeButton );

            pluginEditorWindow->setContentNonOwned( editor, true );
            pluginEditorWindow->setUsingNativeTitleBar( true );
            pluginEditorWindow->setResizable( editor->isResizable(), false );
            pluginEditorWindow->centreWithSize( editor->getWidth(), editor->getHeight() );
            pluginEditorWindow->setVisible( true );
        }
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon, "No Editor Available",
            "This plugin does not have a graphical editor interface." );
    }
}

void MainComponent::closePluginEditor()
{
    if ( pluginEditorWindow != nullptr )
    {
        pluginEditorWindow->clearContentComponent();
        pluginEditorWindow.reset();
    }

    if ( pluginProcessor != nullptr && pluginProcessor->getPluginManager().hasPluginLoaded() )
    {
        auto* plugin = pluginProcessor->getPluginManager().getLoadedPlugin();
        if ( plugin != nullptr && plugin->hasEditor() )
        {
            plugin->editorBeingDeleted( plugin->getActiveEditor() );
        }
    }
}

void MainComponent::toggleWebServer()
{
    if ( webServer == nullptr )
        return;

    if ( webServer->isRunning() )
    {
        webServer->stop();
        webServerPanel->setServerStatus( false, "Web server: Stopped" );

        infoLabel.setText( "Web server stopped", juce::dontSendNotification );
        infoLabel.setColour( juce::Label::textColourId, juce::Colours::grey );
    }
    else
    {
        if ( webServer->start( 8080 ) )
        {
            webServerPanel->setServerStatus( true, "Web server: " + webServer->getServerURL() );

            infoLabel.setText(
                "Web server started! Click 'Open Web UI' or visit " + webServer->getServerURL(),
                juce::dontSendNotification );
            infoLabel.setColour( juce::Label::textColourId, juce::Colours::lightgreen );
        }
        else
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon, "Web Server Error",
                "Failed to start web server on port 8080.\nMake sure the port is not already in use." );

            webServerPanel->setServerStatus( false, "Web server: Failed to start" );
        }
    }
}

void MainComponent::openWebInterface()
{
    if ( webServer != nullptr && webServer->isRunning() )
    {
        juce::URL url( webServer->getServerURL() );
        url.launchInDefaultBrowser();
    }
}
