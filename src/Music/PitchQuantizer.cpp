#include "PitchQuantizer.h"

#include "PitchConverter.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

QuantizedPitch PitchQuantizer::quantize(float midiNote, int rootNote, ScaleType scaleType)
{
    QuantizedPitch result;

    if (!std::isfinite(midiNote))
        return result;

    rootNote = ((rootNote % 12) + 12) % 12;
    const int centreNote = static_cast<int>(std::round(midiNote));
    float smallestDistance = std::numeric_limits<float>::max();

    for (int candidate = centreNote - 12; candidate <= centreNote + 12; ++candidate)
    {
        if (!isNoteAllowed(candidate, rootNote, scaleType))
            continue;

        const float distance = std::abs(static_cast<float>(candidate) - midiNote);
        if (distance < smallestDistance)
        {
            smallestDistance = distance;
            result.targetMidiNote = candidate;
        }
    }

    result.correctionSemitones = static_cast<float>(result.targetMidiNote) - midiNote;
    result.correctionCents = result.correctionSemitones * 100.0f;
    result.targetFrequency = 440.0f
        * std::pow(2.0f, (static_cast<float>(result.targetMidiNote) - 69.0f) / 12.0f);
    result.targetNoteName = PitchConverter::fromFrequency(result.targetFrequency).noteName;
    result.isValid = true;

    return result;
}

bool PitchQuantizer::isNoteAllowed(int midiNote, int rootNote, ScaleType scaleType)
{
    if (scaleType == ScaleType::chromatic)
        return true;

    static constexpr std::array<int, 7> majorIntervals { 0, 2, 4, 5, 7, 9, 11 };
    static constexpr std::array<int, 7> minorIntervals { 0, 2, 3, 5, 7, 8, 10 };

    const int pitchClass = ((midiNote % 12) + 12) % 12;
    const int intervalFromRoot = ((pitchClass - rootNote) + 12) % 12;
    const auto& intervals = scaleType == ScaleType::major ? majorIntervals : minorIntervals;

    return std::find(intervals.begin(), intervals.end(), intervalFromRoot) != intervals.end();
}
