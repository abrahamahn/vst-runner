/**
 * VST Runner Test Runner
 *
 * Runs all registered JUCE unit tests.
 *
 * Usage: VSTRunnerTests [--category <category>] [--test <testname>]
 *
 * Examples:
 *   VSTRunnerTests                    // Run all tests
 *   VSTRunnerTests --category Web     // Run all Web tests
 *   VSTRunnerTests --category Core    // Run all Core tests
 *   VSTRunnerTests --test WebAPIHandler  // Run specific test
 */

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

// Include all test files
#include "Web/WebAPIHandlerTests.cpp"
#include "Core/AudioIOManagerTests.cpp"
#include "Core/PluginManagerTests.cpp"

class TestRunnerApplication : public juce::JUCEApplication
{
public:
    TestRunnerApplication() {}

    const juce::String getApplicationName() override { return "VST Runner Tests"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }

    void initialise (const juce::String& commandLine) override
    {
        // Create and run the test runner
        juce::UnitTestRunner runner;

        // Parse command line arguments
        juce::StringArray args;
        args.addTokens (commandLine, true);

        juce::String categoryFilter;
        juce::String testFilter;

        for (int i = 0; i < args.size(); ++i)
        {
            if (args[i] == "--category" && i + 1 < args.size())
            {
                categoryFilter = args[i + 1];
                ++i;
            }
            else if (args[i] == "--test" && i + 1 < args.size())
            {
                testFilter = args[i + 1];
                ++i;
            }
            else if (args[i] == "--help" || args[i] == "-h")
            {
                printUsage();
                quit();
                return;
            }
        }

        // Print header
        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "  VST Runner Unit Test Suite\n";
        std::cout << "========================================\n";
        std::cout << "\n";

        if (categoryFilter.isNotEmpty())
        {
            std::cout << "Running tests in category: " << categoryFilter << "\n\n";
            runner.runTestsInCategory (categoryFilter);
        }
        else if (testFilter.isNotEmpty())
        {
            std::cout << "Running test: " << testFilter << "\n\n";

            // Find and run specific test
            bool found = false;
            for (auto* test : juce::UnitTest::getAllTests())
            {
                if (test->getName().contains (testFilter))
                {
                    runner.runTests ({ test });
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                std::cout << "Error: Test '" << testFilter << "' not found.\n\n";
                printAvailableTests();
                setApplicationReturnValue (1);
            }
        }
        else
        {
            std::cout << "Running all tests...\n\n";
            runner.runAllTests();
        }

        int failedTests = 0;
        for (int i = 0; i < runner.getNumResults(); ++i)
        {
            if (const auto* result = runner.getResult (i))
            {
                if (result->failures > 0)
                    ++failedTests;
            }
        }

        // Print summary
        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "  Test Results Summary\n";
        std::cout << "========================================\n";
        std::cout << "Tests run:    " << runner.getNumResults() << "\n";
        std::cout << "Passed:       " << (runner.getNumResults() - failedTests) << "\n";
        std::cout << "Failed:       " << failedTests << "\n";
        std::cout << "\n";

        if (failedTests > 0)
        {
            std::cout << "FAILED - Some tests did not pass.\n";
            setApplicationReturnValue (1);
        }
        else
        {
            std::cout << "SUCCESS - All tests passed!\n";
            setApplicationReturnValue (0);
        }

        std::cout << "========================================\n";
        std::cout << "\n";

        quit();
    }

    void shutdown() override
    {
        // Cleanup if needed
    }

    void printUsage()
    {
        std::cout << "\n";
        std::cout << "Usage: VSTRunnerTests [options]\n";
        std::cout << "\n";
        std::cout << "Options:\n";
        std::cout << "  --category <name>    Run tests in specific category (Core, Web, UI)\n";
        std::cout << "  --test <name>        Run specific test by name\n";
        std::cout << "  --help, -h           Show this help message\n";
        std::cout << "\n";
        std::cout << "Examples:\n";
        std::cout << "  VSTRunnerTests                    // Run all tests\n";
        std::cout << "  VSTRunnerTests --category Web     // Run all Web tests\n";
        std::cout << "  VSTRunnerTests --test AudioIO     // Run AudioIOManager tests\n";
        std::cout << "\n";

        printAvailableTests();
    }

    void printAvailableTests()
    {
        std::cout << "Available tests:\n";

        juce::StringArray categories;

        for (auto* test : juce::UnitTest::getAllTests())
        {
            std::cout << "  - " << test->getName()
                      << " (category: " << test->getCategory() << ")\n";

            if (!categories.contains (test->getCategory()))
                categories.add (test->getCategory());
        }

        std::cout << "\nAvailable categories: ";
        for (int i = 0; i < categories.size(); ++i)
        {
            std::cout << categories[i];
            if (i < categories.size() - 1)
                std::cout << ", ";
        }
        std::cout << "\n\n";
    }

    void systemRequestedQuit() override
    {
        quit();
    }
};

// Create application instance
START_JUCE_APPLICATION (TestRunnerApplication)
