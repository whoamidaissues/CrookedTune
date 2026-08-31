#pragma once

#include <JuceHeader.h>

struct PitchInfo
{
    float frequency = 0.0f;
    float midiNote = 0.0f;
    int nearestMidiNote = 0;
    float cents = 0.0f;
    juce::String noteName;
    bool isValid = false;
};

class PitchConverter
{
public:
    static PitchInfo fromFrequency(float frequency);
};
