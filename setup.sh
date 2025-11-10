#!/bin/bash
set -e

# VST Runner Development Environment Setup Script
# For Linux and macOS
# Usage: ./setup.sh

echo "========================================"
echo "  VST Runner Development Setup"
echo "========================================"
echo ""

# Detect OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
    echo "Detected OS: Linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
    echo "Detected OS: macOS"
else
    echo "Unsupported OS: $OSTYPE"
    exit 1
fi

echo ""

# Step 1: Check prerequisites
echo "Step 1: Checking prerequisites..."
echo "-----------------------------------"

# Check CMake
if ! command -v cmake &> /dev/null; then
    echo "❌ CMake not found!"
    echo "Please install CMake 3.15 or higher:"
    if [ "$OS" == "linux" ]; then
        echo "  sudo apt-get install cmake"
    else
        echo "  brew install cmake"
    fi
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
echo "✅ CMake $CMAKE_VERSION"

# Check Git
if ! command -v git &> /dev/null; then
    echo "❌ Git not found! Please install Git."
    exit 1
fi

GIT_VERSION=$(git --version | cut -d' ' -f3)
echo "✅ Git $GIT_VERSION"

# Check C++ compiler
if [ "$OS" == "linux" ]; then
    if ! command -v g++ &> /dev/null; then
        echo "❌ g++ not found! Please install build-essential:"
        echo "  sudo apt-get install build-essential"
        exit 1
    fi
    CXX_VERSION=$(g++ --version | head -n1)
    echo "✅ $CXX_VERSION"
else
    if ! command -v clang++ &> /dev/null; then
        echo "❌ clang++ not found! Please install Xcode Command Line Tools:"
        echo "  xcode-select --install"
        exit 1
    fi
    CXX_VERSION=$(clang++ --version | head -n1)
    echo "✅ $CXX_VERSION"
fi

echo ""

# Step 2: Install system dependencies
echo "Step 2: Installing system dependencies..."
echo "------------------------------------------"

if [ "$OS" == "linux" ]; then
    echo "Checking for JUCE dependencies..."

    MISSING_DEPS=()

    # Check for each required package
    for pkg in libasound2-dev libjack-jackd2-dev libfreetype6-dev libx11-dev libxext-dev libxrandr-dev libxinerama-dev libxcursor-dev libgl1-mesa-dev libglu1-mesa-dev; do
        if ! dpkg -l | grep -q "^ii  $pkg"; then
            MISSING_DEPS+=($pkg)
        fi
    done

    if [ ${#MISSING_DEPS[@]} -gt 0 ]; then
        echo "Installing missing dependencies: ${MISSING_DEPS[*]}"
        echo "You may be prompted for your password..."
        sudo apt-get update
        sudo apt-get install -y ${MISSING_DEPS[@]}
        echo "✅ Dependencies installed"
    else
        echo "✅ All dependencies already installed"
    fi
else
    echo "✅ macOS dependencies managed by Xcode"
fi

# Check for optional tools
if command -v clang-format &> /dev/null; then
    echo "✅ clang-format available"
else
    echo "⚠️  clang-format not found (optional for code formatting)"
    if [ "$OS" == "linux" ]; then
        echo "   Install with: sudo apt-get install clang-format"
    else
        echo "   Install with: brew install clang-format"
    fi
fi

echo ""

# Step 3: Initialize Git submodules
echo "Step 3: Initializing Git submodules..."
echo "---------------------------------------"

if [ -d "JUCE/.git" ]; then
    echo "✅ JUCE submodule already initialized"
else
    echo "Initializing JUCE submodule..."
    git submodule update --init --recursive
    echo "✅ JUCE submodule initialized"
fi

echo ""

# Step 4: Set up pre-commit hooks
echo "Step 4: Setting up pre-commit hooks..."
echo "---------------------------------------"

if [ -f ".git/hooks/pre-commit" ]; then
    echo "⚠️  Pre-commit hook already exists, skipping"
else
    echo "Installing pre-commit hook..."
    cp scripts/pre-commit .git/hooks/pre-commit
    chmod +x .git/hooks/pre-commit
    echo "✅ Pre-commit hook installed"
fi

echo ""

# Step 5: Configure build
echo "Step 5: Configuring build..."
echo "----------------------------"

BUILD_DIR="build"

if [ -d "$BUILD_DIR" ]; then
    echo "⚠️  Build directory already exists"
    read -p "Remove and reconfigure? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        rm -rf "$BUILD_DIR"
        echo "Removed existing build directory"
    else
        echo "Keeping existing build directory"
    fi
fi

if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"

    cd "$BUILD_DIR"

    echo "Running CMake..."
    cmake .. \
        -DCMAKE_BUILD_TYPE=Debug \
        -DBUILD_TESTS=ON \
        -DBUILD_BENCHMARKS=ON

    cd ..
    echo "✅ Build configured"
else
    echo "✅ Build already configured"
fi

echo ""

# Step 6: Build project
echo "Step 6: Building project..."
echo "---------------------------"

read -p "Build now? (Y/n): " -n 1 -r
echo

if [[ ! $REPLY =~ ^[Nn]$ ]]; then
    echo "Building..."
    cmake --build "$BUILD_DIR" --config Debug -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    echo "✅ Build complete"
else
    echo "⏭️  Skipping build"
fi

echo ""

# Step 7: Run tests
echo "Step 7: Running tests..."
echo "------------------------"

read -p "Run tests now? (Y/n): " -n 1 -r
echo

if [[ ! $REPLY =~ ^[Nn]$ ]]; then
    if [ -f "$BUILD_DIR/VSTRunnerTests_artefacts/Debug/VSTRunnerTests" ]; then
        echo "Running test suite..."
        "$BUILD_DIR/VSTRunnerTests_artefacts/Debug/VSTRunnerTests"
    else
        echo "⚠️  Tests not built yet. Build the project first."
    fi
else
    echo "⏭️  Skipping tests"
fi

echo ""
echo "========================================"
echo "  Setup Complete!"
echo "========================================"
echo ""
echo "Next steps:"
echo ""
echo "  Build the project:"
echo "    make build"
echo "    or: cmake --build build"
echo ""
echo "  Run tests:"
echo "    make test"
echo "    or: ./build/VSTRunnerTests_artefacts/Debug/VSTRunnerTests"
echo ""
echo "  Run benchmarks:"
echo "    make benchmark"
echo "    or: ./build/VSTRunnerBenchmarks_artefacts/Release/VSTRunnerBenchmarks"
echo ""
echo "  Format code:"
echo "    make format"
echo "    or: clang-format -i Source/**/*.{h,cpp}"
echo ""
echo "  See all available commands:"
echo "    make help"
echo ""
echo "Happy coding! 🎵"
echo ""
