#pragma once

#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>

/**
    UI panel for web server control.

    Handles:
    - Start/stop web server
    - Open web UI button
    - Server status display
*/
class WebServerPanel : public juce::Component
{
public:
    //==============================================================================
    WebServerPanel();
    ~WebServerPanel() override;

    //==============================================================================
    void paint( juce::Graphics& ) override;
    void resized() override;

    //==============================================================================
    // Callbacks (set by parent component)

    std::function<void()> onToggleWebServer;
    std::function<void()> onOpenWebUI;

    //==============================================================================
    // Update methods (called by parent)

    /** Update server status text and button states */
    void setServerStatus( bool isRunning, const juce::String& statusText );

private:
    //==============================================================================
    void webServerButtonClicked();
    void openWebButtonClicked();

    //==============================================================================
    // UI Components
    juce::TextButton webServerButton;
    juce::TextButton openWebButton;
    juce::Label webServerLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( WebServerPanel )
};
