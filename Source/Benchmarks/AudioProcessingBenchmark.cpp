/**
 * Audio Processing Performance Benchmarks
 *
 * Measures:
 * - Audio buffer processing latency
 * - Plugin load time
 * - Gain application performance
 * - Thread safety overhead
 *
 * Usage: ./VSTRunnerBenchmarks
 */

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "../PluginHostProcessor.h"
#include "../Core/AudioIOManager.h"
#include <chrono>
#include <iostream>
#include <iomanip>

class BenchmarkRunner
{
public:
    using Clock = std::chrono::high_resolution_clock;
    using Duration = std::chrono::duration<double, std::milli>;

    static void run()
    {
        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "  VST Runner Performance Benchmarks\n";
        std::cout << "========================================\n";
        std::cout << "\n";

        benchmarkAudioBufferProcessing();
        benchmarkGainApplication();
        benchmarkAudioIOManager();
        benchmarkProcessorCreation();

        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "  Benchmarks Complete\n";
        std::cout << "========================================\n";
        std::cout << "\n";
    }

private:
    static void benchmarkAudioBufferProcessing()
    {
        std::cout << "Benchmark: Audio Buffer Processing\n";
        std::cout << "-----------------------------------\n";

        const int numChannels = 2;
        const int bufferSizes[] = { 64, 128, 256, 512, 1024, 2048 };

        for (int bufferSize : bufferSizes)
        {
            juce::AudioBuffer<float> buffer (numChannels, bufferSize);
            buffer.clear();

            // Fill buffer with test signal
            for (int ch = 0; ch < numChannels; ++ch)
            {
                auto* channelData = buffer.getWritePointer (ch);
                for (int i = 0; i < bufferSize; ++i)
                {
                    channelData[i] = std::sin (2.0f * juce::MathConstants<float>::pi * 440.0f * i / 44100.0f);
                }
            }

            // Benchmark buffer processing
            const int iterations = 10000;
            auto start = Clock::now();

            for (int i = 0; i < iterations; ++i)
            {
                // Simulate processing: apply gain
                buffer.applyGain (0.8f);
            }

            auto end = Clock::now();
            Duration duration = end - start;

            double avgTime = duration.count() / iterations;
            double throughput = (bufferSize * numChannels * iterations) / (duration.count() / 1000.0);

            std::cout << "  Buffer size: " << std::setw(4) << bufferSize
                      << " | Avg time: " << std::fixed << std::setprecision(4)
                      << avgTime << " ms"
                      << " | Throughput: " << std::setprecision(2)
                      << (throughput / 1000000.0) << " M samples/sec\n";
        }

        std::cout << "\n";
    }

    static void benchmarkGainApplication()
    {
        std::cout << "Benchmark: Gain Application\n";
        std::cout << "----------------------------\n";

        AudioIOManager manager;
        const int bufferSize = 512;
        const int numChannels = 2;
        juce::AudioBuffer<float> buffer (numChannels, bufferSize);

        // Fill buffer with test signal
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* channelData = buffer.getWritePointer (ch);
            for (int i = 0; i < bufferSize; ++i)
            {
                channelData[i] = 1.0f;
            }
        }

        const float gains[] = { 0.0f, 0.5f, 1.0f, 1.5f, 2.0f };
        const int iterations = 50000;

        for (float gain : gains)
        {
            manager.setInputGain (gain);

            auto start = Clock::now();

            for (int i = 0; i < iterations; ++i)
            {
                manager.applyInputGain (buffer);
            }

            auto end = Clock::now();
            Duration duration = end - start;

            double avgTime = duration.count() / iterations;

            std::cout << "  Gain: " << std::fixed << std::setprecision(1) << gain
                      << "x | Avg time: " << std::setprecision(4)
                      << avgTime << " ms\n";
        }

        std::cout << "\n";
    }

    static void benchmarkAudioIOManager()
    {
        std::cout << "Benchmark: AudioIOManager Operations\n";
        std::cout << "-------------------------------------\n";

        AudioIOManager manager;
        const int iterations = 1000000;

        // Benchmark getter performance
        {
            auto start = Clock::now();

            for (int i = 0; i < iterations; ++i)
            {
                volatile float gain = manager.getInputGain();
                (void) gain; // Prevent optimization
            }

            auto end = Clock::now();
            Duration duration = end - start;

            std::cout << "  getInputGain() x" << iterations << ": "
                      << std::fixed << std::setprecision(4)
                      << duration.count() << " ms ("
                      << std::setprecision(6)
                      << (duration.count() / iterations) << " ms/call)\n";
        }

        // Benchmark setter performance
        {
            auto start = Clock::now();

            for (int i = 0; i < iterations; ++i)
            {
                manager.setInputGain (static_cast<float>(i % 100) / 50.0f);
            }

            auto end = Clock::now();
            Duration duration = end - start;

            std::cout << "  setInputGain() x" << iterations << ": "
                      << std::fixed << std::setprecision(4)
                      << duration.count() << " ms ("
                      << std::setprecision(6)
                      << (duration.count() / iterations) << " ms/call)\n";
        }

        // Benchmark toggle performance
        {
            auto start = Clock::now();

            for (int i = 0; i < iterations; ++i)
            {
                manager.setAudioInputEnabled (i % 2 == 0);
            }

            auto end = Clock::now();
            Duration duration = end - start;

            std::cout << "  setAudioInputEnabled() x" << iterations << ": "
                      << std::fixed << std::setprecision(4)
                      << duration.count() << " ms ("
                      << std::setprecision(6)
                      << (duration.count() / iterations) << " ms/call)\n";
        }

        std::cout << "\n";
    }

    static void benchmarkProcessorCreation()
    {
        std::cout << "Benchmark: Processor Creation\n";
        std::cout << "------------------------------\n";

        const int iterations = 1000;

        auto start = Clock::now();

        for (int i = 0; i < iterations; ++i)
        {
            auto processor = std::make_unique<PluginHostProcessor>();
            // Let it destroy
        }

        auto end = Clock::now();
        Duration duration = end - start;

        std::cout << "  PluginHostProcessor creation x" << iterations << ": "
                  << std::fixed << std::setprecision(4)
                  << duration.count() << " ms ("
                  << std::setprecision(4)
                  << (duration.count() / iterations) << " ms/instance)\n";

        std::cout << "\n";
    }
};

class BenchmarkApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "VST Runner Benchmarks"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }

    void initialise (const juce::String&) override
    {
        BenchmarkRunner::run();
        quit();
    }

    void shutdown() override {}
};

START_JUCE_APPLICATION (BenchmarkApplication)
