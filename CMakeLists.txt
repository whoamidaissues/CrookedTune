cmake_minimum_required(VERSION 3.24)

project(CrookedTune VERSION 0.1.0 LANGUAGES C CXX)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CROOKEDTUNE_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND CROOKEDTUNE_FORMATS AU)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/signalsmith-linear signalsmith-linear)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/signalsmith-stretch signalsmith-stretch)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/JUCE JUCE)

set_property(TARGET signalsmith-linear PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/external/signalsmith-linear/include)
set_property(TARGET signalsmith-stretch PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/external/signalsmith-stretch/include)

juce_add_plugin(CrookedTune
    COMPANY_NAME "CrookedTune"
    PLUGIN_MANUFACTURER_CODE Crwd
    PLUGIN_CODE Crtn
    FORMATS ${CROOKEDTUNE_FORMATS}
    PRODUCT_NAME "CrookedTune"
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE
    IS_SYNTH FALSE)

juce_generate_juce_header(CrookedTune)

target_sources(CrookedTune
    PRIVATE
        src/PluginProcessor.cpp
        src/PluginEditor.cpp
        src/DSP/Gain.cpp
        src/DSP/Distortion.cpp
        src/DSP/DryWetMixer.cpp
        src/DSP/PitchDetector.cpp
        src/DSP/PitchShifter.cpp
        src/Music/PitchConverter.cpp
        src/Music/PitchQuantizer.cpp
        src/Music/RetuneController.cpp
        src/PluginProcessor.h
        src/PluginEditor.h
        src/DSP/Gain.h
        src/DSP/Distortion.h
        src/DSP/DryWetMixer.h
        src/DSP/PitchDetector.h
        src/DSP/PitchShifter.h
        src/Music/PitchConverter.h
        src/Music/PitchQuantizer.h
        src/Music/RetuneController.h
)

target_include_directories(CrookedTune PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_definitions(CrookedTune
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(CrookedTune
    PRIVATE
        signalsmith-stretch
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
