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

    // Setup plugin path components
    pluginPathLabel.setText ("Plugin Path:", juce::dontSendNotification);
    addAndMakeVisible (pluginPathLabel);

    pluginPathEditor.setMultiLine (false);
    pluginPathEditor.setReturnKeyStartsNewLine (false);
    pluginPathEditor.setReadOnly (false);
    pluginPathEditor.setScrollbarsShown (true);
    pluginPathEditor.setCaretVisible (true);
    pluginPathEditor.setPopupMenuEnabled (true);
    pluginPathEditor.setTextToShowWhenEmpty ("Enter plugin path or click Browse...", juce::Colours::grey);
    addAndMakeVisible (pluginPathEditor);

    browseButton.setButtonText ("Browse...");
    browseButton.onClick = [this] { browseForPlugin(); };
    addAndMakeVisible (browseButton);

    loadButton.setButtonText ("Load Plugin");
    loadButton.onClick = [this] { loadPluginFromPath (pluginPathEditor.getText()); };
    addAndMakeVisible (loadButton);

    // Setup status label
    statusLabel.setText ("No plugin loaded", juce::dontSendNotification);
    statusLabel.setJustificationType (juce::Justification::centred);
    statusLabel.setColour (juce::Label::textColourId, juce::Colours::orange);
    addAndMakeVisible (statusLabel);

    // Setup audio settings button
    audioSettingsButton.setButtonText ("Audio Settings");
    audioSettingsButton.onClick = [this] { openAudioSettings(); };
    addAndMakeVisible (audioSettingsButton);

    // Setup show editor button
    showEditorButton.setButtonText ("Show Plugin Editor");
    showEditorButton.onClick = [this] { showPluginEditor(); };
    showEditorButton.setEnabled (false);
    addAndMakeVisible (showEditorButton);

    // Setup input gain control
    inputGainLabel.setText ("Input Gain", juce::dontSendNotification);
    inputGainLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (inputGainLabel);

    inputGainSlider.setRange (0.0, 2.0, 0.01);
    inputGainSlider.setValue (1.0);
    inputGainSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    inputGainSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    inputGainSlider.setTextValueSuffix (" x");
    inputGainSlider.onValueChange = [this] {
        if (pluginProcessor != nullptr)
            pluginProcessor->setInputGain (static_cast<float> (inputGainSlider.getValue()));
    };
    addAndMakeVisible (inputGainSlider);

    // Setup output gain control
    outputGainLabel.setText ("Output Gain", juce::dontSendNotification);
    outputGainLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (outputGainLabel);

    outputGainSlider.setRange (0.0, 2.0, 0.01);
    outputGainSlider.setValue (1.0);
    outputGainSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    outputGainSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    outputGainSlider.setTextValueSuffix (" x");
    outputGainSlider.onValueChange = [this] {
        if (pluginProcessor != nullptr)
            pluginProcessor->setOutputGain (static_cast<float> (outputGainSlider.getValue()));
    };
    addAndMakeVisible (outputGainSlider);

    // Setup audio/MIDI input toggles
    audioInputToggle.setButtonText ("Audio Input Enabled");
    audioInputToggle.setToggleState (true, juce::dontSendNotification);
    audioInputToggle.onClick = [this] {
        if (pluginProcessor != nullptr)
            pluginProcessor->setAudioInputEnabled (audioInputToggle.getToggleState());
    };
    addAndMakeVisible (audioInputToggle);

    midiInputToggle.setButtonText ("MIDI Input Enabled");
    midiInputToggle.setToggleState (true, juce::dontSendNotification);
    midiInputToggle.onClick = [this] {
        if (pluginProcessor != nullptr)
            pluginProcessor->setMidiInputEnabled (midiInputToggle.getToggleState());
    };
    addAndMakeVisible (midiInputToggle);

    // Setup info label
    infoLabel.setText ("Load a VST2 (.dll/.vst) or VST3 (.vst3) plugin to begin", juce::dontSendNotification);
    infoLabel.setJustificationType (juce::Justification::centred);
    infoLabel.setFont (juce::Font (12.0f, juce::Font::italic));
    infoLabel.setColour (juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible (infoLabel);

    // Start timer for UI updates
    startTimer (100); // Update every 100ms

    setSize (700, 500);
}

MainComponent::~MainComponent()
{
    // Clean up
    stopTimer();

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

    // Plugin path area
    auto pathArea = area.removeFromTop (30);
    pluginPathLabel.setBounds (pathArea.removeFromLeft (100));
    pathArea.removeFromLeft (5);
    browseButton.setBounds (pathArea.removeFromRight (100));
    pathArea.removeFromRight (5);
    loadButton.setBounds (pathArea.removeFromRight (100));
    pathArea.removeFromRight (5);
    pluginPathEditor.setBounds (pathArea);

    area.removeFromTop (15);

    // Status
    statusLabel.setBounds (area.removeFromTop (30));
    area.removeFromTop (10);

    // Buttons
    auto buttonArea = area.removeFromTop (40);
    audioSettingsButton.setBounds (buttonArea.removeFromLeft (150));
    buttonArea.removeFromLeft (10);
    showEditorButton.setBounds (buttonArea.removeFromLeft (180));

    area.removeFromTop (20);

    // Input gain
    auto inputGainArea = area.removeFromTop (30);
    inputGainLabel.setBounds (inputGainArea.removeFromLeft (100));
    inputGainSlider.setBounds (inputGainArea);

    area.removeFromTop (10);

    // Output gain
    auto outputGainArea = area.removeFromTop (30);
    outputGainLabel.setBounds (outputGainArea.removeFromLeft (100));
    outputGainSlider.setBounds (outputGainArea);

    area.removeFromTop (20);

    // Toggles
    audioInputToggle.setBounds (area.removeFromTop (30));
    area.removeFromTop (5);
    midiInputToggle.setBounds (area.removeFromTop (30));

    area.removeFromTop (20);

    // Info
    infoLabel.setBounds (area.removeFromTop (40));
}

//==============================================================================
void MainComponent::timerCallback()
{
    // Update UI based on plugin state
    if (pluginProcessor != nullptr && pluginProcessor->hasPluginLoaded())
    {
        statusLabel.setText ("Loaded: " + pluginProcessor->getLoadedPluginName(),
                           juce::dontSendNotification);
        statusLabel.setColour (juce::Label::textColourId, juce::Colours::green);
        showEditorButton.setEnabled (true);
    }
    else
    {
        statusLabel.setText ("No plugin loaded", juce::dontSendNotification);
        statusLabel.setColour (juce::Label::textColourId, juce::Colours::orange);
        showEditorButton.setEnabled (false);
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
        pluginPathEditor.setText (file.getFullPathName());
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
