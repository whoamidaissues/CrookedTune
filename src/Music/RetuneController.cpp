#include "RetuneController.h"

#include <cmath>

void RetuneController::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
    currentTargetMidiNote = 0;
    hasCurrentTarget = false;
    currentCorrectionCents = 0.0f;
    publishState(false, 0.0f);
}

void RetuneController::process(float detectedMidiNote,
                               const QuantizedPitch& quantizedPitch,
                               float strengthPercent,
                               float retuneTimeMs,
                               int numSamples,
                               bool forceTargetChange)
{
    if (!quantizedPitch.isValid || !std::isfinite(detectedMidiNote))
    {
        hasCurrentTarget = false;
        currentCorrectionCents = 0.0f;
        publishState(false, 0.0f);
        return;
    }

    const int candidateTarget = quantizedPitch.targetMidiNote;

    if (!hasCurrentTarget || forceTargetChange)
    {
        currentTargetMidiNote = candidateTarget;
        hasCurrentTarget = true;
    }
    else if (candidateTarget != currentTargetMidiNote)
    {
        const float distanceToCurrent = std::abs(static_cast<float>(currentTargetMidiNote) - detectedMidiNote);
        const float distanceToCandidate = std::abs(static_cast<float>(candidateTarget) - detectedMidiNote);

        if (distanceToCandidate + targetHysteresisSemitones < distanceToCurrent)
            currentTargetMidiNote = candidateTarget;
    }

    const float rawCorrectionCents =
        (static_cast<float>(currentTargetMidiNote) - detectedMidiNote) * 100.0f;
    const float strength = juce::jlimit(0.0f, 1.0f, strengthPercent / 100.0f);
    const float desiredCorrectionCents = rawCorrectionCents * strength;
    const float clampedRetuneTimeMs = juce::jmax(0.0f, retuneTimeMs);

    if (clampedRetuneTimeMs <= 0.0f)
    {
        currentCorrectionCents = desiredCorrectionCents;
    }
    else
    {
        const double timeSeconds = static_cast<double>(clampedRetuneTimeMs) / 1000.0;
        const float blockCoefficient = static_cast<float>(
            std::exp(-static_cast<double>(numSamples) / (timeSeconds * sampleRate)));
        currentCorrectionCents = desiredCorrectionCents
            + (currentCorrectionCents - desiredCorrectionCents) * blockCoefficient;
    }

    publishState(true, rawCorrectionCents);
}

RetuneState RetuneController::getState() const
{
    RetuneState state;
    state.targetMidiNote = publishedTargetMidiNote.load();
    state.rawCorrectionCents = publishedRawCorrectionCents.load();
    state.appliedCorrectionCents = publishedAppliedCorrectionCents.load();
    state.isValid = publishedIsValid.load();
    return state;
}

void RetuneController::publishState(bool isValid, float rawCorrectionCents)
{
    publishedTargetMidiNote.store(currentTargetMidiNote);
    publishedRawCorrectionCents.store(rawCorrectionCents);
    publishedAppliedCorrectionCents.store(currentCorrectionCents);
    publishedIsValid.store(isValid);
}
