#include "PitchConverter.h"

#include <array>
#include <cmath>

PitchInfo PitchConverter::fromFrequency(float frequency)
{
    PitchInfo result;

    if (frequency <= 0.0f || !std::isfinite(frequency))
        return result;

    constexpr float concertA = 440.0f;
    constexpr int concertAMidiNote = 69;
    static constexpr std::array<const char*, 12> noteNames {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };

    result.frequency = frequency;
    result.midiNote = static_cast<float>(concertAMidiNote)
                    + 12.0f * std::log2(frequency / concertA);
    result.nearestMidiNote = static_cast<int>(std::round(result.midiNote));
    result.cents = (result.midiNote - static_cast<float>(result.nearestMidiNote)) * 100.0f;

    const int noteIndex = ((result.nearestMidiNote % 12) + 12) % 12;
    const int octave = result.nearestMidiNote / 12 - 1;
    result.noteName = juce::String(noteNames[static_cast<std::size_t>(noteIndex)])
                    + juce::String(octave);
    result.isValid = true;

    return result;
}
