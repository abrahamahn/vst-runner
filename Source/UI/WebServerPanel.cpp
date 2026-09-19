#include "WebServerPanel.h"

//==============================================================================
WebServerPanel::WebServerPanel()
{
    // Setup web server button
    webServerButton.setButtonText ("Start Web Server");
    webServerButton.onClick = [this] { webServerButtonClicked(); };
    addAndMakeVisible (webServerButton);

    // Setup open web button
    openWebButton.setButtonText ("Open Web UI");
    openWebButton.onClick = [this] { openWebButtonClicked(); };
    openWebButton.setEnabled (false);
    addAndMakeVisible (openWebButton);

    // Setup web server status label
    webServerLabel.setText ("Web server: Stopped", juce::dontSendNotification);
    webServerLabel.setJustificationType (juce::Justification::centred);
    webServerLabel.setFont (juce::Font (11.0f));
    webServerLabel.setColour (juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible (webServerLabel);
}

WebServerPanel::~WebServerPanel()
{
}

//==============================================================================
void WebServerPanel::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // Draw a subtle border
    g.setColour (juce::Colours::grey.withAlpha (0.3f));
    g.drawRect (getLocalBounds(), 1);
}

void WebServerPanel::resized()
{
    auto area = getLocalBounds().reduced (10);

    // Web server button
    webServerButton.setBounds (area.removeFromTop (30));

    area.removeFromTop (10);

    // Open web button
    openWebButton.setBounds (area.removeFromTop (30));

    area.removeFromTop (10);

    // Status label
    webServerLabel.setBounds (area.removeFromTop (25));
}

//==============================================================================
void WebServerPanel::webServerButtonClicked()
{
    if (onToggleWebServer)
        onToggleWebServer();
}

void WebServerPanel::openWebButtonClicked()
{
    if (onOpenWebUI)
        onOpenWebUI();
}

//==============================================================================
void WebServerPanel::setServerStatus (bool isRunning, const juce::String& statusText)
{
    webServerButton.setButtonText (isRunning ? "Stop Web Server" : "Start Web Server");
    openWebButton.setEnabled (isRunning);
    webServerLabel.setText (statusText, juce::dontSendNotification);

    // Update label color based on state
    webServerLabel.setColour (juce::Label::textColourId,
                               isRunning ? juce::Colours::green : juce::Colours::grey);
}
