cmake_minimum_required(VERSION 3.15)

project(VSTRunner VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_STATIC "Build with static linking for portable single-file executable" OFF)
option(BUILD_INSTALLER "Build installer package" OFF)

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
    # Application entry point
    Source/Main.cpp

    # Core business logic
    Source/Core/ConfigurationManager.cpp
    Source/Core/ConfigurationManager.h
    Source/Core/PluginManager.cpp
    Source/Core/PluginManager.h
    Source/Core/AudioIOManager.cpp
    Source/Core/AudioIOManager.h

    # UI components
    Source/UI/MainComponent.cpp
    Source/UI/MainComponent.h
    Source/UI/AudioSettingsComponent.cpp
    Source/UI/AudioSettingsComponent.h

    # Audio processing
    Source/PluginHostProcessor.cpp
    Source/PluginHostProcessor.h

    # Web interface
    Source/Web/WebServer.cpp
    Source/Web/WebServer.h
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
        Source/Core
        Source/UI
        Source/Web
        Source/Utils
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

# Static linking for portable executable
if(BUILD_STATIC)
    if(WIN32)
        # Static linking on Windows
        set_target_properties(VSTRunner PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
        )
        target_link_options(VSTRunner PRIVATE
            /INCREMENTAL:NO
            /LTCG
        )
    elseif(UNIX AND NOT APPLE)
        # Static linking on Linux (more challenging - link what we can)
        target_link_options(VSTRunner PRIVATE
            -static-libgcc
            -static-libstdc++
        )
    endif()

    message(STATUS "Building with static linking for portable executable")
endif()

# Installation rules
install(TARGETS VSTRunner
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

# Install web resources
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Source/Web/Resources
    DESTINATION share/vstrunner
    OPTIONAL
)

# CPack configuration for installer generation
if(BUILD_INSTALLER)
    set(CPACK_PACKAGE_NAME "VST Runner")
    set(CPACK_PACKAGE_VENDOR "VSTRunner")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Standalone VST Plugin Host")
    set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "VST Runner")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

    # Platform-specific installer settings
    if(WIN32)
        # Windows: NSIS installer
        set(CPACK_GENERATOR "NSIS;ZIP")
        set(CPACK_NSIS_DISPLAY_NAME "VST Runner")
        set(CPACK_NSIS_PACKAGE_NAME "VST Runner")
        set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
        set(CPACK_NSIS_MODIFY_PATH ON)
        set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon.ico")
        set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon.ico")
    elseif(APPLE)
        # macOS: DMG
        set(CPACK_GENERATOR "DragNDrop;TGZ")
        set(CPACK_DMG_VOLUME_NAME "VST Runner")
        set(CPACK_DMG_FORMAT "UDZO")
    else()
        # Linux: DEB, RPM, TGZ
        set(CPACK_GENERATOR "DEB;RPM;TGZ")
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER "VSTRunner")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libasound2, libfreetype6, libx11-6, libxext6, libxrandr2")
        set(CPACK_RPM_PACKAGE_LICENSE "MIT")
        set(CPACK_RPM_PACKAGE_REQUIRES "alsa-lib, freetype, libX11, libXext, libXrandr")
    endif()

    include(CPack)
    message(STATUS "Installer generation enabled")
endif()
