cmake_minimum_required(VERSION 3.22)

project(SovereignArchonSuite VERSION 6.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure global multi-format plugin settings
set(JUCE_BUILD_FORMATS_VST3 TRUE CACHE INTERNAL "")

# Include the core audio framework dependency directory
add_subdirectory(JUCE)

# Initialize the Unified Plugin Framework Target
juce_add_plugin(SovereignArchonSuite
    VERSION "6.0.0"
    ICON_BIG ""
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE "UESP"
    PLUGIN_CODE "SVRN"
    FORMATS VST3)

# SUCCESS CORRECTION: Pulling files directly from the root workspace directory
target_sources(SovereignArchonSuite PRIVATE
    PluginProcessor.cpp
    PluginProcessor.h
    PluginEditor.cpp
    PluginEditor.h)

target_compile_definitions(SovereignArchonSuite PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0)

target_link_libraries(SovereignArchonSuite PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_extra
    juce::juce_gui_basics)
