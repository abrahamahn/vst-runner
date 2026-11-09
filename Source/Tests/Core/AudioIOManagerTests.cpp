#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <thread>

#include "../../Core/AudioIOManager.h"

/**
 * Unit tests for AudioIOManager
 *
 * Tests cover:
 * - Input gain control
 * - Output gain control
 * - Audio input enable/disable
 * - MIDI input enable/disable
 * - Default values
 * - Boundary conditions
 */
class AudioIOManagerTests : public juce::UnitTest
{
public:
    AudioIOManagerTests() : juce::UnitTest( "AudioIOManager", "Core" ) {}

    void runTest() override
    {
        beginTest( "Default Values" );
        {
            AudioIOManager manager;

            expectWithinAbsoluteError( manager.getInputGain(), 1.0f, 0.0001f, "Default input gain should be 1.0" );
            expectWithinAbsoluteError( manager.getOutputGain(), 1.0f, 0.0001f, "Default output gain should be 1.0" );
            expect( manager.isAudioInputEnabled() == true, "Audio input should be enabled by default" );
            expect( manager.isMidiInputEnabled() == true, "MIDI input should be enabled by default" );
        }

        beginTest( "Set Input Gain - Valid Range" );
        {
            AudioIOManager manager;

            float testGains[] = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 1.25f, 1.5f, 1.75f, 2.0f };

            for ( float gain : testGains )
            {
                manager.setInputGain( gain );
                expectWithinAbsoluteError(
                    manager.getInputGain(), gain, 0.0001f, "Input gain should be set correctly" );
            }
        }

        beginTest( "Set Output Gain - Valid Range" );
        {
            AudioIOManager manager;

            float testGains[] = { 0.0f, 0.5f, 1.0f, 1.5f, 2.0f };

            for ( float gain : testGains )
            {
                manager.setOutputGain( gain );
                expectWithinAbsoluteError(
                    manager.getOutputGain(), gain, 0.0001f, "Output gain should be set correctly" );
            }
        }

        beginTest( "Set Input Gain - Clamping" );
        {
            AudioIOManager manager;

            manager.setInputGain( -1.0f );
            expect( manager.getInputGain() >= 0.0f, "Negative gain should be clamped to minimum" );

            manager.setInputGain( 10.0f );
            expect( manager.getInputGain() <= 2.0f, "Excessive gain should be clamped to maximum" );
        }

        beginTest( "Set Output Gain - Clamping" );
        {
            AudioIOManager manager;

            manager.setOutputGain( -0.5f );
            expect( manager.getOutputGain() >= 0.0f, "Negative gain should be clamped to minimum" );

            manager.setOutputGain( 5.0f );
            expect( manager.getOutputGain() <= 2.0f, "Excessive gain should be clamped to maximum" );
        }

        beginTest( "Audio Input Enable/Disable" );
        {
            AudioIOManager manager;

            manager.setAudioInputEnabled( false );
            expect( !manager.isAudioInputEnabled(), "Audio input should be disabled" );

            manager.setAudioInputEnabled( true );
            expect( manager.isAudioInputEnabled(), "Audio input should be enabled" );

            for ( int i = 0; i < 5; ++i )
            {
                manager.setAudioInputEnabled( i % 2 == 0 );
                expect( manager.isAudioInputEnabled() == ( i % 2 == 0 ), "Audio input state should toggle correctly" );
            }
        }

        beginTest( "MIDI Input Enable/Disable" );
        {
            AudioIOManager manager;

            manager.setMidiInputEnabled( false );
            expect( !manager.isMidiInputEnabled(), "MIDI input should be disabled" );

            manager.setMidiInputEnabled( true );
            expect( manager.isMidiInputEnabled(), "MIDI input should be enabled" );

            for ( int i = 0; i < 5; ++i )
            {
                manager.setMidiInputEnabled( i % 2 == 1 );
                expect( manager.isMidiInputEnabled() == ( i % 2 == 1 ), "MIDI input state should toggle correctly" );
            }
        }

        beginTest( "Apply Input Gain to Buffer" );
        {
            AudioIOManager manager;
            manager.setInputGain( 0.5f );

            juce::AudioBuffer<float> buffer( 2, 512 );
            buffer.clear();

            for ( int ch = 0; ch < buffer.getNumChannels(); ++ch )
            {
                auto* channelData = buffer.getWritePointer( ch );
                for ( int i = 0; i < buffer.getNumSamples(); ++i )
                {
                    channelData[i] = 1.0f;
                }
            }

            manager.processInputGain( buffer );

            for ( int ch = 0; ch < buffer.getNumChannels(); ++ch )
            {
                auto* channelData = buffer.getReadPointer( ch );
                for ( int i = 0; i < buffer.getNumSamples(); ++i )
                {
                    expectWithinAbsoluteError(
                        channelData[i], 0.5f, 0.0001f, "Input gain should be applied to buffer" );
                }
            }
        }

        beginTest( "Apply Output Gain to Buffer" );
        {
            AudioIOManager manager;
            manager.setOutputGain( 2.0f );

            juce::AudioBuffer<float> buffer( 2, 512 );
            buffer.clear();

            for ( int ch = 0; ch < buffer.getNumChannels(); ++ch )
            {
                auto* channelData = buffer.getWritePointer( ch );
                for ( int i = 0; i < buffer.getNumSamples(); ++i )
                {
                    channelData[i] = 0.5f;
                }
            }

            manager.processOutputGain( buffer );

            for ( int ch = 0; ch < buffer.getNumChannels(); ++ch )
            {
                auto* channelData = buffer.getReadPointer( ch );
                for ( int i = 0; i < buffer.getNumSamples(); ++i )
                {
                    expectWithinAbsoluteError(
                        channelData[i], 1.0f, 0.0001f, "Output gain should be applied to buffer" );
                }
            }
        }

        beginTest( "Input Disabled - Buffer Cleared" );
        {
            AudioIOManager manager;
            manager.setAudioInputEnabled( false );

            juce::AudioBuffer<float> buffer( 2, 512 );
            for ( int ch = 0; ch < buffer.getNumChannels(); ++ch )
            {
                auto* channelData = buffer.getWritePointer( ch );
                for ( int i = 0; i < buffer.getNumSamples(); ++i )
                {
                    channelData[i] = 1.0f;
                }
            }

            manager.processInputGain( buffer );

            for ( int ch = 0; ch < buffer.getNumChannels(); ++ch )
            {
                auto* channelData = buffer.getReadPointer( ch );
                for ( int i = 0; i < buffer.getNumSamples(); ++i )
                {
                    expectWithinAbsoluteError(
                        channelData[i], 0.0f, 0.0001f, "Buffer should be cleared when input is disabled" );
                }
            }
        }

        beginTest( "Thread Safety - Concurrent Access" );
        {
            AudioIOManager manager;

            std::atomic<bool> running( true );
            std::atomic<int> numErrors( 0 );

            auto readerThread = [&]()
            {
                while ( running )
                {
                    float inputGain = manager.getInputGain();
                    float outputGain = manager.getOutputGain();
                    bool audioEnabled = manager.isAudioInputEnabled();
                    bool midiEnabled = manager.isMidiInputEnabled();
                    juce::ignoreUnused( audioEnabled, midiEnabled );

                    if ( inputGain < 0.0f || inputGain > 2.0f )
                        ++numErrors;
                    if ( outputGain < 0.0f || outputGain > 2.0f )
                        ++numErrors;
                }
            };

            auto writerThread = [&]()
            {
                for ( int i = 0; i < 100; ++i )
                {
                    manager.setInputGain( static_cast<float>( i % 100 ) / 50.0f );
                    manager.setOutputGain( static_cast<float>( ( i + 50 ) % 100 ) / 50.0f );
                    manager.setAudioInputEnabled( i % 2 == 0 );
                    manager.setMidiInputEnabled( i % 3 == 0 );
                    juce::Thread::sleep( 1 );
                }
            };

            std::thread reader( readerThread );
            std::thread writer( writerThread );

            writer.join();
            running = false;
            reader.join();

            expect( numErrors.load() == 0, "No invalid values should be read during concurrent access" );
        }
    }
};

static AudioIOManagerTests audioIOManagerTests;
