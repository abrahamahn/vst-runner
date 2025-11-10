# Contributing to VST Runner

Thank you for your interest in contributing to VST Runner! This document provides guidelines and instructions for contributing.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Workflow](#development-workflow)
- [Coding Standards](#coding-standards)
- [Testing Guidelines](#testing-guidelines)
- [Pull Request Process](#pull-request-process)
- [Commit Message Guidelines](#commit-message-guidelines)

## Code of Conduct

We are committed to providing a welcoming and inclusive environment. Please be respectful and professional in all interactions.

## Getting Started

### Prerequisites

- **CMake** 3.15 or higher
- **C++17** compatible compiler (MSVC 2019+, Xcode 11+, GCC 9+, Clang 10+)
- **Git** for version control
- **JUCE** framework (included as submodule)

### Setting Up Development Environment

1. **Fork the repository**
   ```bash
   # Fork on GitHub, then clone your fork
   git clone https://github.com/YOUR_USERNAME/vst-runner.git
   cd vst-runner
   ```

2. **Initialize submodules**
   ```bash
   git submodule update --init --recursive
   ```

3. **Create a feature branch**
   ```bash
   git checkout -b feature/your-feature-name
   # or
   git checkout -b fix/bug-description
   ```

4. **Build the project**
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
   cmake --build .
   ```

5. **Run tests**
   ```bash
   ./VSTRunnerTests_artefacts/Debug/VSTRunnerTests
   ```

## Development Workflow

### Branch Naming Convention

- **Features**: `feature/description-here`
- **Bug Fixes**: `fix/bug-description`
- **Refactoring**: `refactor/what-changed`
- **Documentation**: `docs/what-changed`
- **Tests**: `test/what-added`

Examples:
- `feature/add-plugin-chain`
- `fix/audio-dropout-on-buffer-change`
- `refactor/extract-preset-manager`

### Making Changes

1. **Write code** following our [coding standards](#coding-standards)
2. **Write tests** for new functionality (see [Testing Guidelines](#testing-guidelines))
3. **Update documentation** if you're changing behavior or adding features
4. **Run tests** before committing
   ```bash
   ./VSTRunnerTests_artefacts/Debug/VSTRunnerTests
   ```
5. **Format code** (we use clang-format)
   ```bash
   clang-format -i Source/**/*.{h,cpp}
   ```

### Testing Locally

Before submitting a PR, ensure:

- ✅ All tests pass
- ✅ No compiler warnings
- ✅ Code builds on your platform
- ✅ New features have unit tests
- ✅ Documentation is updated

```bash
# Run all tests
./VSTRunnerTests_artefacts/Debug/VSTRunnerTests

# Run specific category
./VSTRunnerTests_artefacts/Debug/VSTRunnerTests --category Core

# Build release version
cmake --build . --config Release
```

## Coding Standards

### C++ Style

We follow modern C++ best practices with JUCE conventions:

#### General Rules

- **C++ Standard**: C++17
- **Indentation**: 4 spaces (no tabs)
- **Line Length**: Maximum 120 characters
- **Braces**: Opening brace on same line
- **Naming Conventions**:
  - Classes: `PascalCase` (e.g., `PluginManager`)
  - Functions: `camelCase` (e.g., `loadPlugin()`)
  - Variables: `camelCase` (e.g., `pluginPath`)
  - Member variables: `camelCase` (e.g., `pluginProcessor`)
  - Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_BUFFER_SIZE`)

#### Code Examples

**Good:**
```cpp
class PluginManager
{
public:
    bool loadPlugin (const juce::File& pluginFile)
    {
        if (!pluginFile.exists())
            return false;

        // Load plugin logic...
        return true;
    }

private:
    std::unique_ptr<juce::AudioProcessor> loadedPlugin;
    juce::String pluginName;
};
```

**Avoid:**
```cpp
class pluginManager {  // Wrong: should be PascalCase
    bool LoadPlugin(juce::File file) {  // Wrong: should be camelCase, use const ref
        if(!file.exists()) return false;  // Wrong: formatting
        // ...
    }

    juce::AudioProcessor* m_plugin;  // Wrong: avoid raw pointers, no m_ prefix
};
```

### JUCE-Specific Guidelines

- Use JUCE smart pointers (`std::unique_ptr`, `std::shared_ptr`)
- Prefer `juce::String` over `std::string` for JUCE APIs
- Use `juce::var` for JSON/dynamic data
- Follow JUCE naming for callbacks (e.g., `buttonClicked`, `timerCallback`)
- Use `juce::MessageManager::callAsync` for cross-thread UI updates

### Architecture Patterns

Follow the established patterns in the codebase:

- **Manager Pattern**: Separate business logic into focused managers
- **Composition**: Compose components rather than inherit
- **Callbacks**: Use `std::function` for loose coupling
- **Observers**: Use observer pattern for state notifications

See [docs/dev/ARCHITECTURE.md](docs/dev/ARCHITECTURE.md) for details.

### File Organization

```
Source/
├── Core/          # Business logic (managers, processors)
├── UI/            # User interface components
├── Web/           # Web server and API
└── Tests/         # Unit tests
```

- **One class per file** (header + implementation)
- **Matching filenames**: `PluginManager.h` + `PluginManager.cpp`
- **Include guards**: Use `#pragma once`

## Testing Guidelines

### Writing Unit Tests

We use JUCE's built-in `UnitTest` framework. All new features should include tests.

#### Test Structure

```cpp
#include <JuceHeader.h>
#include "../../Core/YourClass.h"

class YourClassTests : public juce::UnitTest
{
public:
    YourClassTests() : juce::UnitTest ("YourClass", "Core") {}

    void runTest() override
    {
        beginTest ("Feature description");
        {
            // Arrange
            YourClass instance;

            // Act
            bool result = instance.someMethod();

            // Assert
            expect (result, "Should return true for valid input");
        }

        beginTest ("Edge case description");
        {
            // Test edge cases...
        }
    }
};

// Register the test
static YourClassTests yourClassTests;
```

#### Test Categories

Organize tests by layer:
- **Core**: Business logic tests
- **Web**: API and web server tests
- **UI**: UI component tests

#### Coverage Goals

- Core managers: 80%+ coverage
- API handlers: 90%+ coverage
- UI panels: 50%+ coverage

### Running Tests

```bash
# All tests
./VSTRunnerTests

# By category
./VSTRunnerTests --category Core
./VSTRunnerTests --category Web

# Specific test
./VSTRunnerTests --test PluginManager
```

## Pull Request Process

### Before Submitting

1. ✅ **Update your branch** with latest main
   ```bash
   git fetch origin
   git rebase origin/main
   ```

2. ✅ **Run tests**
   ```bash
   ./VSTRunnerTests_artefacts/Debug/VSTRunnerTests
   ```

3. ✅ **Check build on multiple platforms** (if possible)

4. ✅ **Update documentation** (if changing behavior)

5. ✅ **Add entry to CHANGELOG.md** (under "Unreleased")

### Submitting a PR

1. **Push your branch**
   ```bash
   git push origin feature/your-feature-name
   ```

2. **Create Pull Request** on GitHub

3. **Fill out the PR template** completely

4. **Link related issues** (e.g., "Fixes #123")

### PR Title Format

Use conventional commit style:

```
feat: Add plugin chain support
fix: Resolve audio dropout on buffer change
refactor: Extract preset manager
docs: Update API documentation
test: Add WebAPIHandler integration tests
```

### PR Description

Include:

- **What**: Brief description of changes
- **Why**: Motivation and context
- **How**: Technical approach (if complex)
- **Testing**: How you tested the changes
- **Screenshots**: For UI changes
- **Breaking Changes**: If any

### Review Process

- PRs require at least one approval
- All tests must pass
- No merge conflicts
- Code follows style guidelines
- Documentation is updated

### After Approval

- Maintainers will merge your PR
- Delete your feature branch after merge

## Commit Message Guidelines

### Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

### Types

- `feat`: New feature
- `fix`: Bug fix
- `refactor`: Code restructuring (no behavior change)
- `test`: Adding/modifying tests
- `docs`: Documentation changes
- `style`: Code formatting (no logic change)
- `perf`: Performance improvements
- `build`: Build system changes

### Examples

**Feature:**
```
feat(plugin): Add VST3 preset management

- Implement preset loading and saving
- Add preset browser UI
- Update WebAPI with preset endpoints

Closes #42
```

**Bug Fix:**
```
fix(audio): Resolve buffer overflow on device change

When audio device changes, buffer size could exceed allocated memory.
Now properly reallocates buffers before device switch.

Fixes #89
```

**Refactoring:**
```
refactor(ui): Extract preset panel from MainComponent

Part of Phase 4 refactoring to modularize UI.
Reduces MainComponent from 450 to 300 lines.

See docs/dev/REFACTORING_PHASE4.md
```

## Questions or Issues?

- **Bug Reports**: Use GitHub Issues with bug report template
- **Feature Requests**: Use GitHub Issues with feature request template
- **Questions**: Start a GitHub Discussion
- **Security Issues**: See SECURITY.md

## License

By contributing, you agree that your contributions will be licensed under the same license as the project (MIT).

See [LICENSE](LICENSE) for details.

---

Thank you for contributing to VST Runner! 🎵
