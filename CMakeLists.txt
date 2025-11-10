cmake_minimum_required(VERSION 3.15)

project(VSTRunner VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE as a subdirectory
# JUCE will be added as a git submodule
add_subdirectory(JUCE)

# Create the main application target
juce_add_gui_app(VSTRunner
    COMPANY_NAME "VSTRunner"
    PRODUCT_NAME "VST Runner"
    VERSION "1.0.0"
    BUNDLE_ID "com.vstrunner.app"
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon.png"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "This app needs microphone access for audio input processing."
)

# Add source files
target_sources(VSTRunner PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/MainComponent.h
    Source/PluginHostProcessor.cpp
    Source/PluginHostProcessor.h
    Source/AudioSettingsComponent.cpp
    Source/AudioSettingsComponent.h
    Source/WebServer.cpp
    Source/WebServer.h
)

# Link JUCE modules
target_link_libraries(VSTRunner
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Compile definitions
target_compile_definitions(VSTRunner
    PRIVATE
        # JUCE configuration
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:VSTRunner,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:VSTRunner,JUCE_VERSION>"

        # Plugin hosting
        JUCE_PLUGINHOST_VST3=1
        JUCE_PLUGINHOST_VST=1
        JUCE_PLUGINHOST_AU=1
        JUCE_PLUGINHOST_LADSPA=0

        # Audio drivers
        JUCE_ASIO=1
        JUCE_WASAPI=1
        JUCE_DIRECTSOUND=1
        JUCE_ALSA=1
        JUCE_JACK=1
        JUCE_BELA=0
        JUCE_USE_ANDROID_OBOE=0
        JUCE_USE_ANDROID_OPENSLES=0

        # Disable modal loops for better plugin compatibility
        JUCE_MODAL_LOOPS_PERMITTED=1

        # Display splash screen (set to 0 if you don't want it)
        JUCE_DISPLAY_SPLASH_SCREEN=0

        # Enable VST2 support (if you have the VST2 SDK)
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Include directories
target_include_directories(VSTRunner
    PRIVATE
        Source
)

# Copy any resources if needed
# This would be where you'd copy icons, etc.
# For now, we'll create a simple resources structure

# Platform-specific settings
if(APPLE)
    # macOS specific settings
    target_compile_definitions(VSTRunner PRIVATE
        JUCE_COREGRAPHICS_DRAW_ASYNC=1
    )
    set_target_properties(VSTRunner PROPERTIES
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
    )
elseif(WIN32)
    # Windows specific settings
    target_compile_definitions(VSTRunner PRIVATE
        _UNICODE
        UNICODE
    )
endif()

# Installation rules
install(TARGETS VSTRunner
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
