#pragma once

#include <JuceHeader.h>

#include <array>
#include <atomic>

class PitchDetector
{
public:
    PitchDetector() = default;

    void prepare(double newSampleRate, int samplesPerBlock);
    void process(const juce::AudioBuffer<float>& buffer);

    float getDetectedFrequency() const;
    float getConfidence() const;

private:
    static constexpr int analysisWindowSize = 2048;
    static constexpr int analysisHopSize = 512;
    static constexpr float minimumFrequency = 80.0f;
    static constexpr float maximumFrequency = 1400.0f;
    static constexpr float yinThreshold = 0.15f;
    static constexpr float minimumRms = 0.005f;

    void analyseWindow();
    void publishResult(float frequency, float confidence);

    std::array<float, analysisWindowSize> ringBuffer {};
    std::array<float, analysisWindowSize> analysisBuffer {};
    std::array<float, analysisWindowSize> differenceBuffer {};

    double sampleRate = 44100.0;
    int writePosition = 0;
    int samplesInBuffer = 0;
    int samplesSinceLastAnalysis = 0;

    float lastAcceptedFrequency = 0.0f;
    float pendingOctaveFrequency = 0.0f;
    int pendingOctaveCount = 0;

    std::atomic<float> detectedFrequency { 0.0f };
    std::atomic<float> confidence { 0.0f };
};
