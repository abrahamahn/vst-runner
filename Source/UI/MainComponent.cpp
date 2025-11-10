#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Create the plugin processor
    pluginProcessor = std::make_unique<PluginHostProcessor>();

    // Initialize audio device manager
    deviceManager.initialiseWithDefaultDevices (2, 2); // 2 inputs, 2 outputs

    // Connect the processor to the audio device
    processorPlayer.setProcessor (pluginProcessor.get());
    deviceManager.addAudioCallback (&processorPlayer);

    // Setup title
    titleLabel.setText ("VST Runner", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (24.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);

    // Create plugin control panel
    pluginPanel = std::make_unique<PluginControlPanel>();
    pluginPanel->onBrowseForPlugin = [this] { browseForPlugin(); };
    pluginPanel->onLoadPlugin = [this] (const juce::String& path) { loadPluginFromPath (path); };
    pluginPanel->onShowPluginEditor = [this] { showPluginEditor(); };
    addAndMakeVisible (pluginPanel.get());

    // Create audio control panel
    audioPanel = std::make_unique<AudioControlPanel>();
    audioPanel->onOpenAudioSettings = [this] { openAudioSettings(); };
    audioPanel->onInputGainChanged = [this] (float gain) {
        if (pluginProcessor != nullptr)
            pluginProcessor->setInputGain (gain);
    };
    audioPanel->onOutputGainChanged = [this] (float gain) {
        if (pluginProcessor != nullptr)
            pluginProcessor->setOutputGain (gain);
    };
    audioPanel->onAudioInputToggled = [this] (bool enabled) {
        if (pluginProcessor != nullptr)
            pluginProcessor->setAudioInputEnabled (enabled);
    };
    audioPanel->onMidiInputToggled = [this] (bool enabled) {
        if (pluginProcessor != nullptr)
            pluginProcessor->setMidiInputEnabled (enabled);
    };
    addAndMakeVisible (audioPanel.get());

    // Create web server panel
    webServerPanel = std::make_unique<WebServerPanel>();
    webServerPanel->onToggleWebServer = [this] { toggleWebServer(); };
    webServerPanel->onOpenWebUI = [this] { openWebInterface(); };
    addAndMakeVisible (webServerPanel.get());

    // Setup info label
    infoLabel.setText ("Load a VST2 (.dll/.vst) or VST3 (.vst3) plugin to begin", juce::dontSendNotification);
    infoLabel.setJustificationType (juce::Justification::centred);
    infoLabel.setFont (juce::Font (12.0f, juce::Font::italic));
    infoLabel.setColour (juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible (infoLabel);

    // Create web server instance
    webServer = std::make_unique<WebServer> (pluginProcessor.get(), &deviceManager);

    // Start timer for UI updates
    startTimer (100); // Update every 100ms

    setSize (700, 600);
}

MainComponent::~MainComponent()
{
    // Clean up
    stopTimer();

    // Stop web server
    if (webServer != nullptr)
    {
        webServer->stop();
        webServer.reset();
    }

    closePluginEditor();

    deviceManager.removeAudioCallback (&processorPlayer);
    processorPlayer.setProcessor (nullptr);

    pluginProcessor = nullptr;
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // Draw border around the main area
    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds().reduced (10), 1);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced (20);

    // Title
    titleLabel.setBounds (area.removeFromTop (40));
    area.removeFromTop (10);

    // Plugin control panel
    pluginPanel->setBounds (area.removeFromTop (200));
    area.removeFromTop (10);

    // Audio control panel
    audioPanel->setBounds (area.removeFromTop (180));
    area.removeFromTop (10);

    // Web server panel
    webServerPanel->setBounds (area.removeFromTop (120));
    area.removeFromTop (15);

    // Info
    infoLabel.setBounds (area.removeFromTop (40));
}

//==============================================================================
void MainComponent::timerCallback()
{
    // Update UI based on plugin state
    if (pluginProcessor != nullptr && pluginProcessor->hasPluginLoaded())
    {
        pluginPanel->setStatusText ("Loaded: " + pluginProcessor->getLoadedPluginName());
        pluginPanel->setEditorButtonEnabled (true);
    }
    else
    {
        pluginPanel->setStatusText ("No plugin loaded");
        pluginPanel->setEditorButtonEnabled (false);
    }
}

void MainComponent::browseForPlugin()
{
    juce::FileChooser chooser ("Select a VST Plugin",
                               juce::File::getSpecialLocation (juce::File::userHomeDirectory),
                              #if JUCE_WINDOWS
                               "*.dll;*.vst3"
                              #elif JUCE_MAC
                               "*.vst;*.vst3"
                              #else
                               "*"
                              #endif
                               );

    if (chooser.browseForFileToOpen())
    {
        auto file = chooser.getResult();
        pluginPanel->setPluginPath (file.getFullPathName());
    }
}

void MainComponent::loadPluginFromPath (const juce::String& path)
{
    if (path.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync (
            juce::AlertWindow::WarningIcon,
            "No Path Specified",
            "Please enter a plugin path or use the Browse button.");
        return;
    }

    juce::File pluginFile (path);

    // Close any existing plugin editor
    closePluginEditor();

    // Load the plugin
    if (pluginProcessor->loadPlugin (pluginFile))
    {
        infoLabel.setText ("Plugin loaded successfully! Click 'Show Plugin Editor' to open it.",
                         juce::dontSendNotification);
        infoLabel.setColour (juce::Label::textColourId, juce::Colours::lightgreen);

        // Automatically show the editor
        showPluginEditor();
    }
    else
    {
        infoLabel.setText ("Failed to load plugin. Check the path and plugin format.",
                         juce::dontSendNotification);
        infoLabel.setColour (juce::Label::textColourId, juce::Colours::red);
    }
}

void MainComponent::openAudioSettings()
{
    juce::DialogWindow::LaunchOptions options;

    auto* audioSettings = new AudioSettingsComponent (deviceManager);

    options.content.setOwned (audioSettings);
    options.dialogTitle = "Audio Settings";
    options.dialogBackgroundColour = getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;

    options.launchAsync();
}

void MainComponent::showPluginEditor()
{
    if (pluginProcessor == nullptr || !pluginProcessor->hasPluginLoaded())
        return;

    auto* plugin = pluginProcessor->getLoadedPlugin();

    if (plugin == nullptr)
        return;

    // Close existing editor window if open
    closePluginEditor();

    // Create plugin editor
    if (plugin->hasEditor())
    {
        auto* editor = plugin->createEditorIfNeeded();

        if (editor != nullptr)
        {
            // Create a window to host the plugin editor
            pluginEditorWindow.reset (new juce::DocumentWindow (
                plugin->getName(),
                getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId),
                juce::DocumentWindow::closeButton
            ));

            pluginEditorWindow->setContentNonOwned (editor, true);
            pluginEditorWindow->setUsingNativeTitleBar (true);
            pluginEditorWindow->setResizable (editor->isResizable(), false);
            pluginEditorWindow->centreWithSize (editor->getWidth(), editor->getHeight());
            pluginEditorWindow->setVisible (true);

            // Handle window close
            pluginEditorWindow->setCloseButtonEnabled (true);
        }
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync (
            juce::AlertWindow::InfoIcon,
            "No Editor Available",
            "This plugin does not have a graphical editor interface.");
    }
}

void MainComponent::closePluginEditor()
{
    if (pluginEditorWindow != nullptr)
    {
        pluginEditorWindow->clearContentComponent();
        pluginEditorWindow.reset();
    }

    // Also delete the editor from the plugin
    if (pluginProcessor != nullptr && pluginProcessor->hasPluginLoaded())
    {
        auto* plugin = pluginProcessor->getLoadedPlugin();
        if (plugin != nullptr && plugin->hasEditor())
        {
            plugin->editorBeingDeleted (plugin->getActiveEditor());
        }
    }
}

void MainComponent::toggleWebServer()
{
    if (webServer == nullptr)
        return;

    if (webServer->isRunning())
    {
        // Stop the server
        webServer->stop();
        webServerPanel->setServerStatus (false, "Web server: Stopped");

        infoLabel.setText ("Web server stopped", juce::dontSendNotification);
        infoLabel.setColour (juce::Label::textColourId, juce::Colours::grey);
    }
    else
    {
        // Start the server
        if (webServer->start (8080))
        {
            webServerPanel->setServerStatus (true, "Web server: " + webServer->getServerURL());

            infoLabel.setText ("Web server started! Click 'Open Web UI' or visit " + webServer->getServerURL(),
                             juce::dontSendNotification);
            infoLabel.setColour (juce::Label::textColourId, juce::Colours::lightgreen);
        }
        else
        {
            juce::AlertWindow::showMessageBoxAsync (
                juce::AlertWindow::WarningIcon,
                "Web Server Error",
                "Failed to start web server on port 8080.\nMake sure the port is not already in use.");

            webServerPanel->setServerStatus (false, "Web server: Failed to start");
        }
    }
}

void MainComponent::openWebInterface()
{
    if (webServer != nullptr && webServer->isRunning())
    {
        // Open the web interface in the default browser
        juce::URL url (webServer->getServerURL());
        url.launchInDefaultBrowser();
    }
}
