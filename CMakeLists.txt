cmake_minimum_required(VERSION 3.22)

project(SovereignArchonSuite VERSION 6.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure global binary output properties
set(JUCE_BUILD_FORMATS_VST3 TRUE CACHE INTERNAL "")
set(JUCE_BUILD_FORMATS_AU TRUE CACHE INTERNAL "")

# Include the external core audio framework framework dependency
include(cmake/JUCEHelper.cmake)

# Link shared custom architecture modules
add_subdirectory(modules/shared_dsp)
add_subdirectory(modules/shared_ui)

# Register all six standalone plugin components
add_subdirectory(plugins/SovereignCompressor)
add_subdirectory(plugins/SovereignSaturation)
add_subdirectory(plugins/SovereignReverb)
add_subdirectory(plugins/SovereignDelay)
add_subdirectory(plugins/SovereignEqualizer)
add_subdirectory(plugins/SovereignMaximizer)
