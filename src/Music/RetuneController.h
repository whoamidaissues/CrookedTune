#pragma once

#include "PitchQuantizer.h"

#include <atomic>

struct RetuneState
{
    int targetMidiNote = 0;
    float rawCorrectionCents = 0.0f;
    float appliedCorrectionCents = 0.0f;
    bool isValid = false;
};

class RetuneController
{
public:
    void prepare(double newSampleRate);
    void process(float detectedMidiNote,
                 const QuantizedPitch& quantizedPitch,
                 float strengthPercent,
                 float retuneTimeMs,
                 int numSamples,
                 bool forceTargetChange);

    RetuneState getState() const;

private:
    static constexpr float targetHysteresisSemitones = 0.15f;

    void publishState(bool isValid, float rawCorrectionCents);

    double sampleRate = 44100.0;
    int currentTargetMidiNote = 0;
    bool hasCurrentTarget = false;
    float currentCorrectionCents = 0.0f;

    std::atomic<int> publishedTargetMidiNote { 0 };
    std::atomic<float> publishedRawCorrectionCents { 0.0f };
    std::atomic<float> publishedAppliedCorrectionCents { 0.0f };
    std::atomic<bool> publishedIsValid { false };
};
