#include "PluginControlPanel.h"

//==============================================================================
PluginControlPanel::PluginControlPanel()
{
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
    browseButton.onClick = [this] { browseButtonClicked(); };
    addAndMakeVisible (browseButton);

    loadButton.setButtonText ("Load Plugin");
    loadButton.onClick = [this] { loadButtonClicked(); };
    addAndMakeVisible (loadButton);

    // Setup status label
    statusLabel.setText ("No plugin loaded", juce::dontSendNotification);
    statusLabel.setJustificationType (juce::Justification::centred);
    statusLabel.setColour (juce::Label::textColourId, juce::Colours::orange);
    addAndMakeVisible (statusLabel);

    // Setup show editor button
    showEditorButton.setButtonText ("Show Plugin Editor");
    showEditorButton.onClick = [this] { showEditorButtonClicked(); };
    showEditorButton.setEnabled (false);
    addAndMakeVisible (showEditorButton);
}

PluginControlPanel::~PluginControlPanel()
{
}

//==============================================================================
void PluginControlPanel::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // Draw a subtle border
    g.setColour (juce::Colours::grey.withAlpha (0.3f));
    g.drawRect (getLocalBounds(), 1);
}

void PluginControlPanel::resized()
{
    auto area = getLocalBounds().reduced (10);

    // Plugin path label and editor
    auto pathRow = area.removeFromTop (30);
    pluginPathLabel.setBounds (pathRow.removeFromLeft (90));
    pathRow.removeFromLeft (5);
    pluginPathEditor.setBounds (pathRow);

    area.removeFromTop (10);

    // Browse and Load buttons
    auto buttonRow = area.removeFromTop (30);
    browseButton.setBounds (buttonRow.removeFromLeft (100));
    buttonRow.removeFromLeft (10);
    loadButton.setBounds (buttonRow.removeFromLeft (100));

    area.removeFromTop (10);

    // Status label
    statusLabel.setBounds (area.removeFromTop (25));

    area.removeFromTop (5);

    // Show editor button
    showEditorButton.setBounds (area.removeFromTop (30));
}

//==============================================================================
void PluginControlPanel::browseButtonClicked()
{
    if (onBrowseForPlugin)
        onBrowseForPlugin();
}

void PluginControlPanel::loadButtonClicked()
{
    if (onLoadPlugin)
        onLoadPlugin (pluginPathEditor.getText());
}

void PluginControlPanel::showEditorButtonClicked()
{
    if (onShowPluginEditor)
        onShowPluginEditor();
}

//==============================================================================
void PluginControlPanel::setPluginPath (const juce::String& path)
{
    pluginPathEditor.setText (path);
}

void PluginControlPanel::setStatusText (const juce::String& status)
{
    statusLabel.setText (status, juce::dontSendNotification);
}

void PluginControlPanel::setEditorButtonEnabled (bool enabled)
{
    showEditorButton.setEnabled (enabled);
}
