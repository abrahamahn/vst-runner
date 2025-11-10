#!/bin/bash

# Build script for Linux
# Usage: ./build-linux.sh [normal|static|installer|both]

BUILD_TYPE=${1:-normal}

echo "========================================"
echo "VST Runner - Linux Build Script"
echo "========================================"
echo ""

# Check for required dependencies
check_dependencies() {
    echo "Checking dependencies..."

    MISSING_DEPS=()

    # Check for required packages
    for pkg in libasound2-dev libfreetype6-dev libx11-dev libxext-dev libxrandr-dev libxinerama-dev libxcursor-dev; do
        if ! dpkg -l | grep -q "^ii  $pkg"; then
            MISSING_DEPS+=($pkg)
        fi
    done

    if [ ${#MISSING_DEPS[@]} -gt 0 ]; then
        echo "Missing dependencies: ${MISSING_DEPS[@]}"
        echo "Install with: sudo apt-get install ${MISSING_DEPS[@]}"
        exit 1
    fi

    echo "All dependencies found!"
    echo ""
}

# Initialize JUCE submodule if needed
if [ ! -f "JUCE/CMakeLists.txt" ]; then
    echo "Initializing JUCE submodule..."
    git submodule update --init --recursive
    echo ""
fi

# Check dependencies
check_dependencies

# Create build directory
mkdir -p build
cd build

case $BUILD_TYPE in
    "normal")
        echo "Building standard executable..."
        cmake -DCMAKE_BUILD_TYPE=Release ..
        cmake --build . -- -j$(nproc)
        echo ""
        echo "Build complete! Executable: build/VSTRunner"
        ;;

    "static")
        echo "Building portable static executable..."
        cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON ..
        cmake --build . -- -j$(nproc)
        echo ""
        echo "Portable build complete! Executable: build/VSTRunner"
        ;;

    "installer")
        echo "Building installer packages (DEB, RPM, TGZ)..."
        cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON -DBUILD_INSTALLER=ON ..
        cmake --build . -- -j$(nproc)
        cpack
        echo ""
        echo "Installer complete! Check build directory for .deb, .rpm, and .tar.gz files"
        ;;

    "both")
        echo "Building portable executable AND installer..."
        cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON -DBUILD_INSTALLER=ON ..
        cmake --build . -- -j$(nproc)
        cpack
        echo ""
        echo "Build complete! VSTRunner executable and packages in build directory"
        ;;

    *)
        echo "Unknown build type: $BUILD_TYPE"
        echo "Usage: ./build-linux.sh [normal|static|installer|both]"
        exit 1
        ;;
esac

cd ..

echo ""
echo "========================================"
echo "Build process finished!"
echo "========================================"
echo ""
echo "To run: ./build/VSTRunner"
