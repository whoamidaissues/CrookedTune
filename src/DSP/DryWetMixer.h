#pragma once

#include <JuceHeader.h>

class DryWetMixer
{
public:
    void prepare(double sampleRate, int maximumBlockSize, int numberOfChannels,
                 int dryDelaySamples);
    void reset();

    void captureDrySignal(const juce::AudioBuffer<float>& input);
    void mixWithWetSignal(juce::AudioBuffer<float>& wetSignal,
                          float mixPercent,
                          bool bypassed);

private:
    juce::AudioBuffer<float> delayedDryBuffer;
    juce::AudioBuffer<float> delayBuffer;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> wetMix;

    int delaySamples = 0;
    int writePosition = 0;
};
