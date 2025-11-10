# VST Runner - Docker build environment for Linux
# This provides a consistent build environment with all dependencies

FROM ubuntu:22.04

LABEL maintainer="VST Runner"
LABEL description="Build environment for VST Runner"

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    # Build tools
    build-essential \
    cmake \
    git \
    pkg-config \
    # JUCE dependencies
    libasound2-dev \
    libjack-jackd2-dev \
    ladspa-sdk \
    libcurl4-openssl-dev \
    libfreetype6-dev \
    libx11-dev \
    libxcomposite-dev \
    libxcursor-dev \
    libxcursor-dev \
    libxext-dev \
    libxinerama-dev \
    libxrandr-dev \
    libxrender-dev \
    libwebkit2gtk-4.0-dev \
    libglu1-mesa-dev \
    mesa-common-dev \
    # Testing and development
    clang-format \
    valgrind \
    gdb \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /vst-runner

# Copy source code
COPY . .

# Initialize submodules
RUN git submodule update --init --recursive

# Build
RUN mkdir -p build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON && \
    cmake --build . --config Release -j$(nproc)

# Run tests
RUN cd build && \
    ./VSTRunnerTests_artefacts/Release/VSTRunnerTests

# Set entrypoint
ENTRYPOINT ["/vst-runner/build/VSTRunner_artefacts/Release/VSTRunner"]

# Build instructions:
# docker build -t vst-runner .
#
# Run instructions:
# docker run -it --rm \
#   --device /dev/snd \
#   -v /tmp/.X11-unix:/tmp/.X11-unix \
#   -e DISPLAY=$DISPLAY \
#   vst-runner
#
# For interactive shell:
# docker run -it --rm vst-runner /bin/bash
