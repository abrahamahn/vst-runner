#include <JuceHeader.h>
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
    AudioIOManagerTests() : juce::UnitTest ("AudioIOManager", "Core") {}

    void runTest() override
    {
        beginTest ("Default Values");
        {
            AudioIOManager manager;

            expect (manager.getInputGain() == 1.0f, "Default input gain should be 1.0");
            expect (manager.getOutputGain() == 1.0f, "Default output gain should be 1.0");
            expect (manager.isAudioInputEnabled() == true, "Audio input should be enabled by default");
            expect (manager.isMidiInputEnabled() == true, "MIDI input should be enabled by default");
        }

        beginTest ("Set Input Gain - Valid Range");
        {
            AudioIOManager manager;

            // Test various valid gain values
            float testGains[] = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 1.25f, 1.5f, 1.75f, 2.0f };

            for (float gain : testGains)
            {
                manager.setInputGain (gain);
                expectWithinAbsoluteError (manager.getInputGain(), gain, 0.0001f,
                    "Input gain should be set correctly");
            }
        }

        beginTest ("Set Output Gain - Valid Range");
        {
            AudioIOManager manager;

            // Test various valid gain values
            float testGains[] = { 0.0f, 0.5f, 1.0f, 1.5f, 2.0f };

            for (float gain : testGains)
            {
                manager.setOutputGain (gain);
                expectWithinAbsoluteError (manager.getOutputGain(), gain, 0.0001f,
                    "Output gain should be set correctly");
            }
        }

        beginTest ("Set Input Gain - Clamping");
        {
            AudioIOManager manager;

            // Test values outside valid range
            manager.setInputGain (-1.0f);
            expect (manager.getInputGain() >= 0.0f, "Negative gain should be clamped to minimum");

            manager.setInputGain (10.0f);
            expect (manager.getInputGain() <= 2.0f, "Excessive gain should be clamped to maximum");
        }

        beginTest ("Set Output Gain - Clamping");
        {
            AudioIOManager manager;

            // Test values outside valid range
            manager.setOutputGain (-0.5f);
            expect (manager.getOutputGain() >= 0.0f, "Negative gain should be clamped to minimum");

            manager.setOutputGain (5.0f);
            expect (manager.getOutputGain() <= 2.0f, "Excessive gain should be clamped to maximum");
        }

        beginTest ("Audio Input Enable/Disable");
        {
            AudioIOManager manager;

            // Test disabling
            manager.setAudioInputEnabled (false);
            expect (!manager.isAudioInputEnabled(), "Audio input should be disabled");

            // Test re-enabling
            manager.setAudioInputEnabled (true);
            expect (manager.isAudioInputEnabled(), "Audio input should be enabled");

            // Test multiple toggles
            for (int i = 0; i < 5; ++i)
            {
                manager.setAudioInputEnabled (i % 2 == 0);
                expect (manager.isAudioInputEnabled() == (i % 2 == 0),
                    "Audio input state should toggle correctly");
            }
        }

        beginTest ("MIDI Input Enable/Disable");
        {
            AudioIOManager manager;

            // Test disabling
            manager.setMidiInputEnabled (false);
            expect (!manager.isMidiInputEnabled(), "MIDI input should be disabled");

            // Test re-enabling
            manager.setMidiInputEnabled (true);
            expect (manager.isMidiInputEnabled(), "MIDI input should be enabled");

            // Test multiple toggles
            for (int i = 0; i < 5; ++i)
            {
                manager.setMidiInputEnabled (i % 2 == 1);
                expect (manager.isMidiInputEnabled() == (i % 2 == 1),
                    "MIDI input state should toggle correctly");
            }
        }

        beginTest ("Apply Input Gain to Buffer");
        {
            AudioIOManager manager;
            manager.setInputGain (0.5f);

            // Create test buffer with known values
            juce::AudioBuffer<float> buffer (2, 512);
            buffer.clear();

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* channelData = buffer.getWritePointer (ch);
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    channelData[i] = 1.0f; // Fill with 1.0
                }
            }

            // Apply input gain
            manager.applyInputGain (buffer);

            // Verify gain was applied
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* channelData = buffer.getReadPointer (ch);
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    expectWithinAbsoluteError (channelData[i], 0.5f, 0.0001f,
                        "Input gain should be applied to buffer");
                }
            }
        }

        beginTest ("Apply Output Gain to Buffer");
        {
            AudioIOManager manager;
            manager.setOutputGain (2.0f);

            // Create test buffer with known values
            juce::AudioBuffer<float> buffer (2, 512);
            buffer.clear();

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* channelData = buffer.getWritePointer (ch);
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    channelData[i] = 0.5f; // Fill with 0.5
                }
            }

            // Apply output gain
            manager.applyOutputGain (buffer);

            // Verify gain was applied
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* channelData = buffer.getReadPointer (ch);
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    expectWithinAbsoluteError (channelData[i], 1.0f, 0.0001f,
                        "Output gain should be applied to buffer");
                }
            }
        }

        beginTest ("Input Disabled - Buffer Cleared");
        {
            AudioIOManager manager;
            manager.setAudioInputEnabled (false);

            // Create test buffer with non-zero values
            juce::AudioBuffer<float> buffer (2, 512);
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* channelData = buffer.getWritePointer (ch);
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    channelData[i] = 1.0f;
                }
            }

            // Apply input gain (should clear when disabled)
            manager.applyInputGain (buffer);

            // Verify buffer was cleared
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* channelData = buffer.getReadPointer (ch);
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    expectWithinAbsoluteError (channelData[i], 0.0f, 0.0001f,
                        "Buffer should be cleared when input is disabled");
                }
            }
        }

        beginTest ("Thread Safety - Concurrent Access");
        {
            AudioIOManager manager;

            // Simulate concurrent reads and writes
            std::atomic<bool> running (true);
            int numErrors = 0;

            auto readerThread = [&]()
            {
                while (running)
                {
                    float inputGain = manager.getInputGain();
                    float outputGain = manager.getOutputGain();
                    bool audioEnabled = manager.isAudioInputEnabled();
                    bool midiEnabled = manager.isMidiInputEnabled();

                    // Verify values are in valid range
                    if (inputGain < 0.0f || inputGain > 2.0f) ++numErrors;
                    if (outputGain < 0.0f || outputGain > 2.0f) ++numErrors;
                }
            };

            auto writerThread = [&]()
            {
                for (int i = 0; i < 100; ++i)
                {
                    manager.setInputGain (static_cast<float>(i % 100) / 50.0f);
                    manager.setOutputGain (static_cast<float>((i + 50) % 100) / 50.0f);
                    manager.setAudioInputEnabled (i % 2 == 0);
                    manager.setMidiInputEnabled (i % 3 == 0);
                    juce::Thread::sleep (1);
                }
            };

            std::thread reader (readerThread);
            std::thread writer (writerThread);

            writer.join();
            running = false;
            reader.join();

            expect (numErrors == 0, "No invalid values should be read during concurrent access");
        }
    }
};

// Register the test
static AudioIOManagerTests audioIOManagerTests;
