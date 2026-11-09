cmake_minimum_required(VERSION 3.22)

project(VSTRunner VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_STATIC "Build with static linking for portable single-file executable" OFF)
option(BUILD_INSTALLER "Build installer package" OFF)
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_BENCHMARKS "Build performance benchmarks" ON)
option(ENABLE_VST2 "Enable legacy VST2 hosting (requires Steinberg VST2 SDK)" OFF)
option(ENABLE_ASIO "Enable ASIO audio driver support on Windows (requires Steinberg ASIO SDK)" OFF)

# Detect optional Linux audio backends
set(VSTRUNNER_ALSA_ENABLED 0)
set(VSTRUNNER_JACK_ENABLED 0)
if(UNIX AND NOT APPLE)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(ALSA QUIET alsa)
        if(ALSA_FOUND)
            set(VSTRUNNER_ALSA_ENABLED 1)
        endif()
        pkg_check_modules(JACK QUIET jack)
        if(JACK_FOUND)
            set(VSTRUNNER_JACK_ENABLED 1)
        endif()
    endif()
endif()

set(VSTRUNNER_VST2_ENABLED 0)
if(ENABLE_VST2)
    set(VSTRUNNER_VST2_ENABLED 1)
endif()

set(VSTRUNNER_ASIO_ENABLED 0)
if(WIN32 AND ENABLE_ASIO)
    set(VSTRUNNER_ASIO_ENABLED 1)
endif()

# Add JUCE submodule
add_subdirectory(JUCE)

# Embed web interface assets as binary resources
juce_add_binary_data(VSTRunnerWebResources
    NAMESPACE WebResources
    SOURCES
        Source/Web/Resources/index.html
        Source/Web/Resources/app.js
)

set(VSTRUNNER_CORE_SOURCES
    Source/Core/ConfigurationManager.cpp
    Source/Core/ConfigurationManager.h
    Source/Core/PluginManager.cpp
    Source/Core/PluginManager.h
    Source/Core/AudioIOManager.cpp
    Source/Core/AudioIOManager.h
    Source/Core/PluginHostProcessor.cpp
    Source/Core/PluginHostProcessor.h
)

set(VSTRUNNER_JUCE_MODULES
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
)

set(VSTRUNNER_COMPILE_DEFINITIONS
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_VST=${VSTRUNNER_VST2_ENABLED}
    JUCE_PLUGINHOST_AU=$<IF:$<BOOL:${APPLE}>,1,0>
    JUCE_PLUGINHOST_LADSPA=0
    JUCE_ASIO=${VSTRUNNER_ASIO_ENABLED}
    JUCE_WASAPI=$<IF:$<BOOL:${WIN32}>,1,0>
    JUCE_DIRECTSOUND=$<IF:$<BOOL:${WIN32}>,1,0>
    JUCE_ALSA=${VSTRUNNER_ALSA_ENABLED}
    JUCE_JACK=${VSTRUNNER_JACK_ENABLED}
    JUCE_BELA=0
    JUCE_USE_ANDROID_OBOE=0
    JUCE_USE_ANDROID_OPENSLES=0
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_VST3_CAN_REPLACE_VST2=0
)

#==============================================================================
# Main Application Target
#==============================================================================
juce_add_gui_app(VSTRunner
    COMPANY_NAME "VSTRunner"
    PRODUCT_NAME "VST Runner"
    VERSION "1.0.0"
    BUNDLE_ID "com.vstrunner.app"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "This app needs microphone access for audio input processing."
)

target_sources(VSTRunner PRIVATE
    Source/Main.cpp
    ${VSTRUNNER_CORE_SOURCES}
    Source/UI/MainComponent.cpp
    Source/UI/MainComponent.h
    Source/UI/AudioSettingsComponent.cpp
    Source/UI/AudioSettingsComponent.h
    Source/UI/PluginControlPanel.cpp
    Source/UI/PluginControlPanel.h
    Source/UI/AudioControlPanel.cpp
    Source/UI/AudioControlPanel.h
    Source/UI/WebServerPanel.cpp
    Source/UI/WebServerPanel.h
    Source/Web/WebServer.cpp
    Source/Web/WebServer.h
    Source/Web/WebAPIHandler.cpp
    Source/Web/WebAPIHandler.h
)

target_link_libraries(VSTRunner
    PRIVATE
        VSTRunnerWebResources
        ${VSTRUNNER_JUCE_MODULES}
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(VSTRunner
    PRIVATE
        ${VSTRUNNER_COMPILE_DEFINITIONS}
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:VSTRunner,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:VSTRunner,JUCE_VERSION>"
)

target_include_directories(VSTRunner
    PRIVATE
        Source
        Source/Core
        Source/UI
        Source/Web
)

if(APPLE)
    target_compile_definitions(VSTRunner PRIVATE JUCE_COREGRAPHICS_DRAW_ASYNC=1)
    set_target_properties(VSTRunner PROPERTIES XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES)
elseif(WIN32)
    target_compile_definitions(VSTRunner PRIVATE _UNICODE UNICODE)
endif()

if(BUILD_STATIC)
    if(WIN32)
        set_target_properties(VSTRunner PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
        )
        target_link_options(VSTRunner PRIVATE /INCREMENTAL:NO /LTCG)
    elseif(UNIX AND NOT APPLE)
        target_link_options(VSTRunner PRIVATE -static-libgcc -static-libstdc++)
    endif()
    message(STATUS "Building with static linking for portable executable")
endif()

install(TARGETS VSTRunner
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Source/Web/Resources
    DESTINATION share/vstrunner
    OPTIONAL
)

if(BUILD_INSTALLER)
    set(CPACK_PACKAGE_NAME "VST Runner")
    set(CPACK_PACKAGE_VENDOR "VSTRunner")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Standalone VST Plugin Host")
    set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "VST Runner")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

    if(WIN32)
        set(CPACK_GENERATOR "NSIS;ZIP")
        set(CPACK_NSIS_DISPLAY_NAME "VST Runner")
        set(CPACK_NSIS_PACKAGE_NAME "VST Runner")
        set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
        set(CPACK_NSIS_MODIFY_PATH ON)
    elseif(APPLE)
        set(CPACK_GENERATOR "DragNDrop;TGZ")
        set(CPACK_DMG_VOLUME_NAME "VST Runner")
        set(CPACK_DMG_FORMAT "UDZO")
    else()
        set(CPACK_GENERATOR "DEB;RPM;TGZ")
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER "VSTRunner")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libasound2, libfreetype6, libx11-6, libxext6, libxrandr2")
        set(CPACK_RPM_PACKAGE_LICENSE "MIT")
        set(CPACK_RPM_PACKAGE_REQUIRES "alsa-lib, freetype, libX11, libXext, libXrandr")
    endif()

    include(CPack)
    message(STATUS "Installer generation enabled")
endif()

#==============================================================================
# Test Runner
#==============================================================================
if(BUILD_TESTS)
    juce_add_console_app(VSTRunnerTests
        PRODUCT_NAME "VST Runner Tests"
    )

    target_sources(VSTRunnerTests PRIVATE
        Source/Tests/TestRunner.cpp
        ${VSTRUNNER_CORE_SOURCES}
        Source/Web/WebAPIHandler.cpp
        Source/Web/WebAPIHandler.h
    )

    target_link_libraries(VSTRunnerTests
        PRIVATE
            ${VSTRUNNER_JUCE_MODULES}
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )

    target_compile_definitions(VSTRunnerTests
        PRIVATE
            ${VSTRUNNER_COMPILE_DEFINITIONS}
    )

    target_include_directories(VSTRunnerTests
        PRIVATE
            Source
            Source/Core
            Source/UI
            Source/Web
            Source/Tests
    )

    message(STATUS "Unit tests enabled - VSTRunnerTests target created")
endif()

#==============================================================================
# Benchmarks
#==============================================================================
if(BUILD_BENCHMARKS)
    juce_add_console_app(VSTRunnerBenchmarks
        PRODUCT_NAME "VST Runner Benchmarks"
    )

    target_sources(VSTRunnerBenchmarks PRIVATE
        Source/Benchmarks/AudioProcessingBenchmark.cpp
        ${VSTRUNNER_CORE_SOURCES}
    )

    target_link_libraries(VSTRunnerBenchmarks
        PRIVATE
            ${VSTRUNNER_JUCE_MODULES}
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )

    target_compile_definitions(VSTRunnerBenchmarks
        PRIVATE
            ${VSTRUNNER_COMPILE_DEFINITIONS}
    )

    target_include_directories(VSTRunnerBenchmarks
        PRIVATE
            Source
            Source/Core
            Source/Benchmarks
    )

    message(STATUS "Performance benchmarks enabled - VSTRunnerBenchmarks target created")
endif()
