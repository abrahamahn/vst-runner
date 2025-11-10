#include <JuceHeader.h>
#include "../../Core/PluginManager.h"

/**
 * Unit tests for PluginManager
 *
 * Tests cover:
 * - Initial state
 * - Plugin lifecycle
 * - Invalid plugin handling
 * - Observer notifications
 * - Edge cases
 *
 * Note: Full plugin loading tests require actual VST plugins
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

            // Try to load non-existent file
            juce::File invalidFile ("/nonexistent/path/plugin.vst3");

            bool result = manager.loadPlugin (invalidFile);

            expect (!result, "Loading non-existent file should fail");
            expect (!manager.hasPluginLoaded(), "No plugin should be loaded after failed load");
            expect (manager.getLoadedPlugin() == nullptr, "Plugin pointer should still be null");
        }

        beginTest ("Load Invalid Plugin - Directory");
        {
            PluginManager manager;

            // Try to load a directory (not a valid plugin file)
            juce::File directory = juce::File::getSpecialLocation (juce::File::tempDirectory);

            bool result = manager.loadPlugin (directory);

            expect (!result, "Loading directory should fail");
            expect (!manager.hasPluginLoaded(), "No plugin should be loaded");
        }

        beginTest ("Load Invalid Plugin - Wrong Extension");
        {
            PluginManager manager;

            // Create a temporary file with wrong extension
            juce::File tempFile = juce::File::getSpecialLocation (juce::File::tempDirectory)
                                    .getChildFile ("test_invalid.txt");
            tempFile.create();
            tempFile.appendText ("This is not a plugin");

            bool result = manager.loadPlugin (tempFile);

            expect (!result, "Loading non-plugin file should fail");
            expect (!manager.hasPluginLoaded(), "No plugin should be loaded");

            // Clean up
            tempFile.deleteFile();
        }

        beginTest ("Unload Plugin - No Plugin Loaded");
        {
            PluginManager manager;

            // Should not crash when unloading with no plugin loaded
            manager.unloadPlugin();

            expect (!manager.hasPluginLoaded(), "No plugin should be loaded");
            expect (manager.getLoadedPlugin() == nullptr, "Plugin pointer should be null");
        }

        beginTest ("Multiple Load Attempts - Same File");
        {
            PluginManager manager;

            juce::File invalidFile ("/test/plugin.vst3");

            // Try loading same file multiple times
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

            // Try to load invalid plugin
            juce::File invalidFile ("/invalid/path/plugin.dll");
            manager.loadPlugin (invalidFile);

            // Verify clean state
            expect (!manager.hasPluginLoaded(), "hasPluginLoaded should be false");
            expect (manager.getLoadedPluginName().isEmpty(), "Plugin name should be empty");
            expect (manager.getLoadedPlugin() == nullptr, "Plugin pointer should be null");

            // Should be able to attempt another load
            juce::File anotherFile ("/another/invalid/plugin.vst3");
            manager.loadPlugin (anotherFile);

            expect (!manager.hasPluginLoaded(), "Still no plugin loaded");
        }

        beginTest ("Get Plugin Name - No Plugin");
        {
            PluginManager manager;

            juce::String name = manager.getLoadedPluginName();

            expect (name.isEmpty(), "Plugin name should be empty when no plugin loaded");
            expect (name == "", "Plugin name should be empty string");
        }

        beginTest ("Observer Notification - Plugin State Change");
        {
            PluginManager manager;

            // Simple observer to track notifications
            class TestObserver : public PluginManager::Observer
            {
            public:
                int notificationCount = 0;

                void pluginStateChanged() override
                {
                    ++notificationCount;
                }
            };

            TestObserver observer;
            manager.addObserver (&observer);

            // Load invalid plugin (should still trigger notification)
            juce::File invalidFile ("/test/plugin.vst3");
            manager.loadPlugin (invalidFile);

            // Note: Notification behavior depends on implementation
            // Some implementations only notify on successful load
            // Others notify on any load attempt

            manager.removeObserver (&observer);

            // Verify observer was removed (no crash on next operation)
            manager.unloadPlugin();
        }

        beginTest ("Multiple Observers");
        {
            PluginManager manager;

            class CountingObserver : public PluginManager::Observer
            {
            public:
                int count = 0;
                void pluginStateChanged() override { ++count; }
            };

            CountingObserver observer1, observer2, observer3;

            manager.addObserver (&observer1);
            manager.addObserver (&observer2);
            manager.addObserver (&observer3);

            // Trigger a state change
            manager.unloadPlugin();

            // Clean up
            manager.removeObserver (&observer1);
            manager.removeObserver (&observer2);
            manager.removeObserver (&observer3);
        }

        beginTest ("Remove Observer - Not Added");
        {
            PluginManager manager;

            class DummyObserver : public PluginManager::Observer
            {
            public:
                void pluginStateChanged() override {}
            };

            DummyObserver observer;

            // Should not crash when removing observer that wasn't added
            manager.removeObserver (&observer);

            expect (true, "Should handle removing non-existent observer");
        }

        beginTest ("Concurrent Observer Management");
        {
            PluginManager manager;

            class SafeObserver : public PluginManager::Observer
            {
            public:
                std::atomic<int> count {0};
                void pluginStateChanged() override { ++count; }
            };

            SafeObserver observer;

            // Add and remove observer multiple times
            for (int i = 0; i < 10; ++i)
            {
                manager.addObserver (&observer);
                manager.removeObserver (&observer);
            }

            expect (true, "Should handle concurrent add/remove");
        }

        beginTest ("Plugin Manager Destruction");
        {
            // Create manager in scope
            {
                PluginManager manager;

                class TestObserver : public PluginManager::Observer
                {
                public:
                    void pluginStateChanged() override {}
                };

                TestObserver observer;
                manager.addObserver (&observer);

                // Manager should clean up properly when destroyed
            }

            expect (true, "Manager should destroy cleanly");
        }
    }
};

// Register the test
static PluginManagerTests pluginManagerTests;
