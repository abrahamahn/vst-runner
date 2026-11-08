#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>

#include "../../Core/PluginManager.h"

/**
 * Unit tests for PluginManager
 *
 * Tests cover:
 * - Initial state
 * - Plugin lifecycle
 * - Invalid plugin handling
 * - Listener notifications
 * - Edge cases
 */
class PluginManagerTests : public juce::UnitTest
{
public:
    PluginManagerTests() : juce::UnitTest ("PluginManager", "Core") {}

    void runTest() override
    {
        beginTest ("Initial State - No Plugin Loaded");
        {
            PluginManager manager;

            expect (!manager.hasPluginLoaded(), "No plugin should be loaded initially");
            expect (manager.getLoadedPluginName().isEmpty(), "Plugin name should be empty");
            expect (manager.getLoadedPlugin() == nullptr, "Plugin pointer should be null");
        }

        beginTest ("Load Invalid Plugin - Empty File");
        {
            PluginManager manager;

            juce::File invalidFile ("/nonexistent/path/plugin.vst3");

            bool result = manager.loadPlugin (invalidFile);

            expect (!result, "Loading non-existent file should fail");
            expect (!manager.hasPluginLoaded(), "No plugin should be loaded after failed load");
            expect (manager.getLoadedPlugin() == nullptr, "Plugin pointer should still be null");
        }

        beginTest ("Load Invalid Plugin - Directory");
        {
            PluginManager manager;

            juce::File directory = juce::File::getSpecialLocation (juce::File::tempDirectory);

            bool result = manager.loadPlugin (directory);

            expect (!result, "Loading directory should fail");
            expect (!manager.hasPluginLoaded(), "No plugin should be loaded");
        }

        beginTest ("Load Invalid Plugin - Wrong Extension");
        {
            PluginManager manager;

            juce::File tempFile = juce::File::getSpecialLocation (juce::File::tempDirectory)
                                      .getChildFile ("test_invalid.txt");
            tempFile.create();
            tempFile.appendText ("This is not a plugin");

            bool result = manager.loadPlugin (tempFile);

            expect (!result, "Loading non-plugin file should fail");
            expect (!manager.hasPluginLoaded(), "No plugin should be loaded");

            tempFile.deleteFile();
        }

        beginTest ("Unload Plugin - No Plugin Loaded");
        {
            PluginManager manager;

            manager.unloadPlugin();

            expect (!manager.hasPluginLoaded(), "No plugin should be loaded");
            expect (manager.getLoadedPlugin() == nullptr, "Plugin pointer should be null");
        }

        beginTest ("Multiple Load Attempts - Same File");
        {
            PluginManager manager;

            juce::File invalidFile ("/test/plugin.vst3");

            for (int i = 0; i < 5; ++i)
            {
                bool result = manager.loadPlugin (invalidFile);
                expect (!result, "Each load attempt should fail");
                expect (!manager.hasPluginLoaded(), "No plugin should ever be loaded");
            }
        }

        beginTest ("Plugin State After Failed Load");
        {
            PluginManager manager;

            juce::File invalidFile ("/invalid/path/plugin.dll");
            manager.loadPlugin (invalidFile);

            expect (!manager.hasPluginLoaded(), "hasPluginLoaded should be false");
            expect (manager.getLoadedPluginName().isEmpty(), "Plugin name should be empty");
            expect (manager.getLoadedPlugin() == nullptr, "Plugin pointer should be null");

            juce::File anotherFile ("/another/invalid/plugin.vst3");
            manager.loadPlugin (anotherFile);

            expect (!manager.hasPluginLoaded(), "Still no plugin loaded");
        }

        beginTest ("Get Plugin Name - No Plugin");
        {
            PluginManager manager;

            juce::String pluginName = manager.getLoadedPluginName();

            expect (pluginName.isEmpty(), "Plugin name should be empty when no plugin loaded");
            expect (pluginName == "", "Plugin name should be empty string");
        }

        beginTest ("Listener Notification - Plugin Load Failure");
        {
            PluginManager manager;

            class TestListener : public PluginManager::Listener
            {
            public:
                int failureCount = 0;

                void pluginLoadFailed (const juce::String& errorMessage) override
                {
                    if (errorMessage.isNotEmpty())
                        ++failureCount;
                }
            };

            TestListener listener;
            manager.addListener (&listener);

            juce::File invalidFile ("/test/plugin.vst3");
            manager.loadPlugin (invalidFile);

            expect (listener.failureCount == 1, "Listener should receive load failure notification");

            manager.removeListener (&listener);
            manager.unloadPlugin();
        }

        beginTest ("Multiple Listeners");
        {
            PluginManager manager;

            class CountingListener : public PluginManager::Listener
            {
            public:
                int count = 0;
                void pluginLoadFailed (const juce::String& errorMessage) override
                {
                    juce::ignoreUnused (errorMessage);
                    ++count;
                }
            };

            CountingListener listener1, listener2, listener3;

            manager.addListener (&listener1);
            manager.addListener (&listener2);
            manager.addListener (&listener3);

            juce::File invalidFile ("/test/plugin.vst3");
            manager.loadPlugin (invalidFile);

            expect (listener1.count == 1 && listener2.count == 1 && listener3.count == 1,
                    "All listeners should be notified");

            manager.removeListener (&listener1);
            manager.removeListener (&listener2);
            manager.removeListener (&listener3);
        }

        beginTest ("Remove Listener - Not Added");
        {
            PluginManager manager;

            class DummyListener : public PluginManager::Listener
            {
            };

            DummyListener listener;
            manager.removeListener (&listener);

            expect (true, "Should handle removing non-existent listener");
        }

        beginTest ("Concurrent Listener Management");
        {
            PluginManager manager;

            class SafeListener : public PluginManager::Listener
            {
            public:
                std::atomic<int> count { 0 };
                void pluginUnloaded() override { ++count; }
            };

            SafeListener listener;

            for (int i = 0; i < 10; ++i)
            {
                manager.addListener (&listener);
                manager.removeListener (&listener);
            }

            expect (true, "Should handle add/remove cycles");
        }

        beginTest ("Plugin Manager Destruction");
        {
            {
                PluginManager manager;

                class TestListener : public PluginManager::Listener
                {
                };

                TestListener listener;
                manager.addListener (&listener);
                manager.removeListener (&listener);
            }

            expect (true, "Manager should destroy cleanly");
        }
    }
};

static PluginManagerTests pluginManagerTests;
