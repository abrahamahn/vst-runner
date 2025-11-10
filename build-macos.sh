#!/bin/bash

# Build script for macOS
# Usage: ./build-macos.sh [normal|installer|both]

BUILD_TYPE=${1:-normal}

echo "========================================"
echo "VST Runner - macOS Build Script"
echo "========================================"
echo ""

# Initialize JUCE submodule if needed
if [ ! -f "JUCE/CMakeLists.txt" ]; then
    echo "Initializing JUCE submodule..."
    git submodule update --init --recursive
    echo ""
fi

# Create build directory
mkdir -p build
cd build

case $BUILD_TYPE in
    "normal")
        echo "Building standard application..."
        cmake -G "Xcode" ..
        cmake --build . --config Release
        echo ""
        echo "Build complete! Application: build/Release/VSTRunner.app"
        ;;

    "installer")
        echo "Building installer package (DMG)..."
        cmake -G "Xcode" -DBUILD_INSTALLER=ON ..
        cmake --build . --config Release
        cpack -C Release
        echo ""
        echo "Installer complete! Check build directory for .dmg file"
        ;;

    "both")
        echo "Building application AND installer..."
        cmake -G "Xcode" -DBUILD_INSTALLER=ON ..
        cmake --build . --config Release
        cpack -C Release
        echo ""
        echo "Build complete! Application and DMG in build directory"
        ;;

    *)
        echo "Unknown build type: $BUILD_TYPE"
        echo "Usage: ./build-macos.sh [normal|installer|both]"
        exit 1
        ;;
esac

cd ..

echo ""
echo "========================================"
echo "Build process finished!"
echo "========================================"
