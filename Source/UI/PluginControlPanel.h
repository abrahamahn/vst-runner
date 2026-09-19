#pragma once

#include <JuceHeader.h>
#include <functional>

/**
    UI panel for plugin control operations.

    Handles:
    - Plugin path input
    - Browse for plugin file
    - Load/unload plugin buttons
    - Plugin status display
    - Show plugin editor button
*/
class PluginControlPanel : public juce::Component
{
public:
    //==============================================================================
    PluginControlPanel();
    ~PluginControlPanel() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    // Callbacks (set by parent component)

    std::function<void()> onBrowseForPlugin;
    std::function<void(const juce::String&)> onLoadPlugin;
    std::function<void()> onShowPluginEditor;

    //==============================================================================
    // Update methods (called by parent)

    /** Update the plugin path editor text */
    void setPluginPath (const juce::String& path);

    /** Update the status label */
    void setStatusText (const juce::String& status);

    /** Enable/disable the "Show Editor" button */
    void setEditorButtonEnabled (bool enabled);

private:
    //==============================================================================
    void browseButtonClicked();
    void loadButtonClicked();
    void showEditorButtonClicked();

    //==============================================================================
    // UI Components
    juce::Label pluginPathLabel;
    juce::TextEditor pluginPathEditor;
    juce::TextButton browseButton;
    juce::TextButton loadButton;
    juce::TextButton showEditorButton;
    juce::Label statusLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginControlPanel)
};
