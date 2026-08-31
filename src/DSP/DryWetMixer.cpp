#include "DryWetMixer.h"

void DryWetMixer::prepare(double sampleRate, int maximumBlockSize,
                          int numberOfChannels, int dryDelaySamples)
{
    delaySamples = juce::jmax(0, dryDelaySamples);
    delayedDryBuffer.setSize(numberOfChannels, maximumBlockSize, false, true, false);
    delayBuffer.setSize(numberOfChannels, juce::jmax(1, delaySamples), false, true, false);

    wetMix.reset(sampleRate, 0.02);
    wetMix.setCurrentAndTargetValue(1.0f);
    reset();
}

void DryWetMixer::reset()
{
    delayedDryBuffer.clear();
    delayBuffer.clear();
    writePosition = 0;
}

void DryWetMixer::captureDrySignal(const juce::AudioBuffer<float>& input)
{
    jassert(input.getNumChannels() <= delayedDryBuffer.getNumChannels());
    jassert(input.getNumSamples() <= delayedDryBuffer.getNumSamples());

    const int channels = juce::jmin(input.getNumChannels(), delayedDryBuffer.getNumChannels());
    const int samples = juce::jmin(input.getNumSamples(), delayedDryBuffer.getNumSamples());

    if (delaySamples == 0)
    {
        for (int channel = 0; channel < channels; ++channel)
            delayedDryBuffer.copyFrom(channel, 0, input, channel, 0, samples);
        return;
    }

    for (int sample = 0; sample < samples; ++sample)
    {
        for (int channel = 0; channel < channels; ++channel)
        {
            auto* delayData = delayBuffer.getWritePointer(channel);
            delayedDryBuffer.setSample(channel, sample, delayData[writePosition]);
            delayData[writePosition] = input.getSample(channel, sample);
        }

        writePosition = (writePosition + 1) % delaySamples;
    }
}

void DryWetMixer::mixWithWetSignal(juce::AudioBuffer<float>& wetSignal,
                                   float mixPercent, bool bypassed)
{
    wetMix.setTargetValue(bypassed ? 0.0f
                                   : juce::jlimit(0.0f, 1.0f, mixPercent * 0.01f));

    const int channels = juce::jmin(wetSignal.getNumChannels(), delayedDryBuffer.getNumChannels());
    const int samples = juce::jmin(wetSignal.getNumSamples(), delayedDryBuffer.getNumSamples());

    for (int sample = 0; sample < samples; ++sample)
    {
        const float wetAmount = wetMix.getNextValue();
        const float dryAmount = 1.0f - wetAmount;

        for (int channel = 0; channel < channels; ++channel)
        {
            const float drySample = delayedDryBuffer.getSample(channel, sample);
            const float wetSample = wetSignal.getSample(channel, sample);
            wetSignal.setSample(channel, sample,
                                drySample * dryAmount + wetSample * wetAmount);
        }
    }
}
