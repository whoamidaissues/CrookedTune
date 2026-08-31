#pragma once

#include <JuceHeader.h>

class Gain
{
public:
    Gain() = default;

    void prepare(double sampleRate);
    void setGain(float newGain);
    void process(juce::AudioBuffer<float>& buffer);
    float getGain() const;

private:
    float targetGain = 1.0f;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedGain;
};
