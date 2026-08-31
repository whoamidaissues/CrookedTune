#pragma once

#include <JuceHeader.h>

enum class ScaleType
{
    chromatic = 0,
    major,
    minor
};

struct QuantizedPitch
{
    int targetMidiNote = 0;
    float targetFrequency = 0.0f;
    float correctionSemitones = 0.0f;
    float correctionCents = 0.0f;
    juce::String targetNoteName;
    bool isValid = false;
};

class PitchQuantizer
{
public:
    static QuantizedPitch quantize(float midiNote, int rootNote, ScaleType scaleType);

private:
    static bool isNoteAllowed(int midiNote, int rootNote, ScaleType scaleType);
};
