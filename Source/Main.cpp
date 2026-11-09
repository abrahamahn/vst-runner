#include "UI/MainComponent.h"
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
class VSTRunnerApplication : public juce::JUCEApplication
{
public:
    //==============================================================================
    VSTRunnerApplication() {}

    const juce::String getApplicationName() override { return JUCE_APPLICATION_NAME_STRING; }
    const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override { return true; }

    //==============================================================================
    void initialise( const juce::String& commandLine ) override
    {
        juce::ignoreUnused( commandLine );
        mainWindow = std::make_unique<MainWindow>( getApplicationName() );
    }

    void shutdown() override { mainWindow = nullptr; }

    //==============================================================================
    void systemRequestedQuit() override { quit(); }

    void anotherInstanceStarted( const juce::String& commandLine ) override { juce::ignoreUnused( commandLine ); }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow( juce::String name )
            : DocumentWindow(
                  name,
                  juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
                      juce::ResizableWindow::backgroundColourId ),
                  DocumentWindow::allButtons )
        {
            setUsingNativeTitleBar( true );
            setContentOwned( new MainComponent(), true );

#if JUCE_IOS || JUCE_ANDROID
            setFullScreen( true );
#else
            setResizable( true, true );
            centreWithSize( getWidth(), getHeight() );
#endif

            setVisible( true );
        }

        void closeButtonPressed() override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( MainWindow )
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION( VSTRunnerApplication )
