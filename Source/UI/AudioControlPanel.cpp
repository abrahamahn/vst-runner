#include "AudioControlPanel.h"

//==============================================================================
AudioControlPanel::AudioControlPanel()
{
    // Setup audio settings button
    audioSettingsButton.setButtonText( "Audio Settings" );
    audioSettingsButton.onClick = [this] { audioSettingsButtonClicked(); };
    addAndMakeVisible( audioSettingsButton );

    // Setup input gain control
    inputGainLabel.setText( "Input Gain", juce::dontSendNotification );
    inputGainLabel.setJustificationType( juce::Justification::centredRight );
    addAndMakeVisible( inputGainLabel );

    inputGainSlider.setRange( 0.0, 2.0, 0.01 );
    inputGainSlider.setValue( 1.0 );
    inputGainSlider.setSliderStyle( juce::Slider::LinearHorizontal );
    inputGainSlider.setTextBoxStyle( juce::Slider::TextBoxRight, false, 60, 20 );
    inputGainSlider.setTextValueSuffix( " x" );
    inputGainSlider.onValueChange = [this]
    {
        if ( onInputGainChanged )
            onInputGainChanged( static_cast<float>( inputGainSlider.getValue() ) );
    };
    addAndMakeVisible( inputGainSlider );

    // Setup output gain control
    outputGainLabel.setText( "Output Gain", juce::dontSendNotification );
    outputGainLabel.setJustificationType( juce::Justification::centredRight );
    addAndMakeVisible( outputGainLabel );

    outputGainSlider.setRange( 0.0, 2.0, 0.01 );
    outputGainSlider.setValue( 1.0 );
    outputGainSlider.setSliderStyle( juce::Slider::LinearHorizontal );
    outputGainSlider.setTextBoxStyle( juce::Slider::TextBoxRight, false, 60, 20 );
    outputGainSlider.setTextValueSuffix( " x" );
    outputGainSlider.onValueChange = [this]
    {
        if ( onOutputGainChanged )
            onOutputGainChanged( static_cast<float>( outputGainSlider.getValue() ) );
    };
    addAndMakeVisible( outputGainSlider );

    // Setup audio/MIDI input toggles
    audioInputToggle.setButtonText( "Audio Input Enabled" );
    audioInputToggle.setToggleState( true, juce::dontSendNotification );
    audioInputToggle.onClick = [this]
    {
        if ( onAudioInputToggled )
            onAudioInputToggled( audioInputToggle.getToggleState() );
    };
    addAndMakeVisible( audioInputToggle );

    midiInputToggle.setButtonText( "MIDI Input Enabled" );
    midiInputToggle.setToggleState( true, juce::dontSendNotification );
    midiInputToggle.onClick = [this]
    {
        if ( onMidiInputToggled )
            onMidiInputToggled( midiInputToggle.getToggleState() );
    };
    addAndMakeVisible( midiInputToggle );
}

AudioControlPanel::~AudioControlPanel() {}

//==============================================================================
void AudioControlPanel::paint( juce::Graphics& g )
{
    g.fillAll( getLookAndFeel().findColour( juce::ResizableWindow::backgroundColourId ) );

    // Draw a subtle border
    g.setColour( juce::Colours::grey.withAlpha( 0.3f ) );
    g.drawRect( getLocalBounds(), 1 );
}

void AudioControlPanel::resized()
{
    auto area = getLocalBounds().reduced( 10 );

    // Audio settings button
    audioSettingsButton.setBounds( area.removeFromTop( 30 ) );

    area.removeFromTop( 15 );

    // Input gain
    auto inputRow = area.removeFromTop( 30 );
    inputGainLabel.setBounds( inputRow.removeFromLeft( 90 ) );
    inputRow.removeFromLeft( 5 );
    inputGainSlider.setBounds( inputRow );

    area.removeFromTop( 10 );

    // Output gain
    auto outputRow = area.removeFromTop( 30 );
    outputGainLabel.setBounds( outputRow.removeFromLeft( 90 ) );
    outputRow.removeFromLeft( 5 );
    outputGainSlider.setBounds( outputRow );

    area.removeFromTop( 15 );

    // Audio input toggle
    audioInputToggle.setBounds( area.removeFromTop( 25 ) );

    area.removeFromTop( 5 );

    // MIDI input toggle
    midiInputToggle.setBounds( area.removeFromTop( 25 ) );
}

//==============================================================================
void AudioControlPanel::audioSettingsButtonClicked()
{
    if ( onOpenAudioSettings )
        onOpenAudioSettings();
}

//==============================================================================
void AudioControlPanel::setInputGain( float gain )
{
    inputGainSlider.setValue( gain, juce::dontSendNotification );
}

void AudioControlPanel::setOutputGain( float gain )
{
    outputGainSlider.setValue( gain, juce::dontSendNotification );
}

void AudioControlPanel::setAudioInputEnabled( bool enabled )
{
    audioInputToggle.setToggleState( enabled, juce::dontSendNotification );
}

void AudioControlPanel::setMidiInputEnabled( bool enabled )
{
    midiInputToggle.setToggleState( enabled, juce::dontSendNotification );
}
