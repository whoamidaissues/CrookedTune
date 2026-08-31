#include "Gain.h"

void Gain::prepare(double sampleRate)
{
    constexpr double smoothingTimeSeconds = 0.02;

    smoothedGain.reset(sampleRate, smoothingTimeSeconds);
    smoothedGain.setCurrentAndTargetValue(targetGain);
}

void Gain::setGain(float newGain)
{
    targetGain = juce::jlimit(0.0f, 4.0f, newGain);
    smoothedGain.setTargetValue(targetGain);
}

void Gain::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const float gainForCurrentSample = smoothedGain.getNextValue();

        for (int channel = 0; channel < numChannels; ++channel)
        {
            buffer.getWritePointer(channel)[sample] *= gainForCurrentSample;
        }
    }
}

float Gain::getGain() const{
    return targetGain;
}
