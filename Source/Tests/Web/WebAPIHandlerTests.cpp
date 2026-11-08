#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_core/juce_core.h>

#include "../../Core/PluginHostProcessor.h"
#include "../../Web/WebAPIHandler.h"

/**
 * Unit tests for WebAPIHandler
 *
 * Tests cover:
 * - JSON response formatting
 * - Error handling
 * - Status endpoint
 * - Plugin info endpoint
 * - Audio settings endpoint
 * - Gain control endpoints
 */
class WebAPIHandlerTests : public juce::UnitTest
{
public:
    WebAPIHandlerTests() : juce::UnitTest ("WebAPIHandler", "Web") {}

    void runTest() override
    {
        beginTest ("JSON Response Formatting");
        {
            auto processor = std::make_unique<PluginHostProcessor>();
            juce::AudioDeviceManager deviceManager;

            WebAPIHandler handler (processor.get(), &deviceManager);

            juce::var testData (new juce::DynamicObject());
            testData.getDynamicObject()->setProperty ("test", "value");
            testData.getDynamicObject()->setProperty ("number", 42);

            juce::String response = handler.jsonResponse (testData);

            auto parsedResponse = juce::JSON::parse (response);
            expect (parsedResponse.isObject(), "Response should be valid JSON object");
            expect (parsedResponse["test"].toString() == "value", "Should contain test property");
            expect ((int) parsedResponse["number"] == 42, "Should contain number property");
        }

        beginTest ("Error Response Formatting");
        {
            auto processor = std::make_unique<PluginHostProcessor>();
            juce::AudioDeviceManager deviceManager;

            WebAPIHandler handler (processor.get(), &deviceManager);

            juce::String errorMsg = "Test error message";
            juce::String response = handler.errorResponse (errorMsg);

            auto parsedResponse = juce::JSON::parse (response);
            expect (parsedResponse.isObject(), "Error response should be valid JSON object");
            expect (parsedResponse.hasProperty ("error"), "Should have error property");
            expect (parsedResponse["error"].toString() == errorMsg, "Error message should match");
        }

        beginTest ("Get Status - No Plugin Loaded");
        {
            auto processor = std::make_unique<PluginHostProcessor>();
            juce::AudioDeviceManager deviceManager;

            WebAPIHandler handler (processor.get(), &deviceManager);

            juce::String response = handler.handleGetStatus();
            auto parsedResponse = juce::JSON::parse (response);

            expect (parsedResponse.isObject(), "Status response should be valid JSON");
            expect (parsedResponse.hasProperty ("pluginLoaded"), "Should have pluginLoaded property");
            expect (!(bool) parsedResponse["pluginLoaded"], "Plugin should not be loaded");
            expect (parsedResponse["pluginName"].toString().isEmpty(), "Plugin name should be empty");
        }

        beginTest ("Get Audio Settings");
        {
            auto processor = std::make_unique<PluginHostProcessor>();
            juce::AudioDeviceManager deviceManager;

            WebAPIHandler handler (processor.get(), &deviceManager);

            juce::String response = handler.handleGetAudioSettings();
            auto parsedResponse = juce::JSON::parse (response);

            expect (parsedResponse.isObject(), "Audio settings should be valid JSON");
            expect (parsedResponse.hasProperty ("inputGain"), "Should have inputGain property");
            expect (parsedResponse.hasProperty ("outputGain"), "Should have outputGain property");
            expect (parsedResponse.hasProperty ("audioInputEnabled"), "Should have audioInputEnabled property");
            expect (parsedResponse.hasProperty ("midiInputEnabled"), "Should have midiInputEnabled property");

            expectWithinAbsoluteError ((float) parsedResponse["inputGain"], 1.0f, 0.0001f, "Default input gain should be 1.0");
            expectWithinAbsoluteError ((float) parsedResponse["outputGain"], 1.0f, 0.0001f, "Default output gain should be 1.0");
            expect ((bool) parsedResponse["audioInputEnabled"] == true, "Audio input should be enabled by default");
            expect ((bool) parsedResponse["midiInputEnabled"] == true, "MIDI input should be enabled by default");
        }

        beginTest ("Set Input Gain - Valid Range");
        {
            auto processor = std::make_unique<PluginHostProcessor>();
            juce::AudioDeviceManager deviceManager;

            WebAPIHandler handler (processor.get(), &deviceManager);

            float testGains[] = { 0.0f, 0.5f, 1.0f, 1.5f, 2.0f };

            for (float gain : testGains)
            {
                juce::String response = handler.handleSetInputGain (gain);
                auto parsedResponse = juce::JSON::parse (response);

                expect (parsedResponse.isObject(), "Response should be valid JSON");
                expect (parsedResponse.hasProperty ("success"), "Should have success property");
                expect ((bool) parsedResponse["success"], "Should succeed for valid gain");

                expectWithinAbsoluteError (processor->getAudioIOManager().getInputGain(), gain, 0.0001f,
                                           "Input gain should be set correctly");
            }
        }

        beginTest ("Set Output Gain - Valid Range");
        {
            auto processor = std::make_unique<PluginHostProcessor>();
            juce::AudioDeviceManager deviceManager;

            WebAPIHandler handler (processor.get(), &deviceManager);

            float testGain = 1.5f;
            juce::String response = handler.handleSetOutputGain (testGain);
            auto parsedResponse = juce::JSON::parse (response);

            expect (parsedResponse.isObject(), "Response should be valid JSON");
            expect (parsedResponse.hasProperty ("success"), "Should have success property");
            expect ((bool) parsedResponse["success"], "Should succeed for valid gain");
            expectWithinAbsoluteError (processor->getAudioIOManager().getOutputGain(), testGain, 0.0001f,
                                       "Output gain should be set correctly");
        }

        beginTest ("Set Audio Input Enable/Disable");
        {
            auto processor = std::make_unique<PluginHostProcessor>();
            juce::AudioDeviceManager deviceManager;

            WebAPIHandler handler (processor.get(), &deviceManager);

            juce::String response = handler.handleSetAudioInput (false);
            auto parsedResponse = juce::JSON::parse (response);

            expect ((bool) parsedResponse["success"], "Should succeed");
            expect (!processor->getAudioIOManager().isAudioInputEnabled(),
                    "Audio input should be disabled");

            response = handler.handleSetAudioInput (true);
            parsedResponse = juce::JSON::parse (response);

            expect ((bool) parsedResponse["success"], "Should succeed");
            expect (processor->getAudioIOManager().isAudioInputEnabled(),
                    "Audio input should be enabled");
        }

        beginTest ("Set MIDI Input Enable/Disable");
        {
            auto processor = std::make_unique<PluginHostProcessor>();
            juce::AudioDeviceManager deviceManager;

            WebAPIHandler handler (processor.get(), &deviceManager);

            juce::String response = handler.handleSetMidiInput (false);
            auto parsedResponse = juce::JSON::parse (response);

            expect ((bool) parsedResponse["success"], "Should succeed");
            expect (!processor->getAudioIOManager().isMidiInputEnabled(),
                    "MIDI input should be disabled");

            response = handler.handleSetMidiInput (true);
            parsedResponse = juce::JSON::parse (response);

            expect ((bool) parsedResponse["success"], "Should succeed");
            expect (processor->getAudioIOManager().isMidiInputEnabled(),
                    "MIDI input should be enabled");
        }

        beginTest ("Get Plugin Info - No Plugin");
        {
            auto processor = std::make_unique<PluginHostProcessor>();
            juce::AudioDeviceManager deviceManager;

            WebAPIHandler handler (processor.get(), &deviceManager);

            juce::String response = handler.handleGetPluginInfo();
            auto parsedResponse = juce::JSON::parse (response);

            expect (parsedResponse.isObject(), "Response should be valid JSON");
            expect (parsedResponse.hasProperty ("loaded"), "Should have loaded property");
            expect (!(bool) parsedResponse["loaded"], "Should indicate no plugin loaded");
        }

        beginTest ("Unload Plugin - No Plugin Loaded");
        {
            auto processor = std::make_unique<PluginHostProcessor>();
            juce::AudioDeviceManager deviceManager;

            WebAPIHandler handler (processor.get(), &deviceManager);

            juce::String response = handler.handleUnloadPlugin();
            auto parsedResponse = juce::JSON::parse (response);

            expect (parsedResponse.isObject(), "Response should be valid JSON");
            expect (parsedResponse.hasProperty ("success"), "Should have success property");
        }

        beginTest ("Load Plugin - Invalid Path");
        {
            auto processor = std::make_unique<PluginHostProcessor>();
            juce::AudioDeviceManager deviceManager;

            WebAPIHandler handler (processor.get(), &deviceManager);

            juce::String response = handler.handleLoadPlugin ("/invalid/path/to/plugin.vst3");
            auto parsedResponse = juce::JSON::parse (response);

            expect (parsedResponse.isObject(), "Response should be valid JSON");
            expect (parsedResponse.hasProperty ("success"), "Should have success property");
            expect (!(bool) parsedResponse["success"], "Should fail for invalid plugin path");
        }

        beginTest ("Scan Plugins - Empty Directory");
        {
            auto processor = std::make_unique<PluginHostProcessor>();
            juce::AudioDeviceManager deviceManager;

            WebAPIHandler handler (processor.get(), &deviceManager);

            juce::File tempDir = juce::File::getSpecialLocation (juce::File::tempDirectory)
                                     .getChildFile ("vst_runner_test_empty");
            tempDir.createDirectory();

            juce::String response = handler.handleScanPlugins (tempDir.getFullPathName());
            auto parsedResponse = juce::JSON::parse (response);

            expect (parsedResponse.isObject(), "Response should be valid JSON");
            expect (parsedResponse.hasProperty ("plugins"), "Should have plugins array");
            expect (parsedResponse["plugins"].isArray(), "Plugins should be an array");

            tempDir.deleteRecursively();
        }
    }
};

static WebAPIHandlerTests webAPIHandlerTests;
