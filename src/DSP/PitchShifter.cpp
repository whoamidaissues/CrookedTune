#include <signalsmith-stretch/signalsmith-stretch.h>

#include "PitchShifter.h"

#include <cmath>

class PitchShifter::Impl
{
public:
    signalsmith::stretch::SignalsmithStretch<float> stretch;
};

PitchShifter::PitchShifter()
    : impl(std::make_unique<Impl>())
{
}

PitchShifter::~PitchShifter() = default;

void PitchShifter::prepare(double newSampleRate, int maximumBlockSize, int numChannels)
{
    sampleRate = newSampleRate;
    preparedChannels = juce::jmax(1, numChannels);
    preparedBlockSize = juce::jmax(1, maximumBlockSize);

    inputBuffer.setSize(preparedChannels, preparedBlockSize, false, false, true);
    outputBuffer.setSize(preparedChannels, preparedBlockSize, false, false, true);

    const int spectralBlockSize = juce::jmax(256, juce::roundToInt(sampleRate * 0.05));
    const int spectralIntervalSize = juce::jmax(64, juce::roundToInt(sampleRate * 0.0125));

    impl->stretch.configure(preparedChannels, spectralBlockSize, spectralIntervalSize, true);
    impl->stretch.setTransposeSemitones(0.0f);
    impl->stretch.setFormantFactor(1.0f, true);
    impl->stretch.setFormantBase(200.0f / static_cast<float>(sampleRate));
    impl->stretch.reset();

    latencySamples = impl->stretch.inputLatency() + impl->stretch.outputLatency();
}

void PitchShifter::reset()
{
    impl->stretch.reset();
    inputBuffer.clear();
    outputBuffer.clear();
}

void PitchShifter::setPitchShiftCents(float newPitchShiftCents)
{
    const float clampedCents = juce::jlimit(-1200.0f, 1200.0f, newPitchShiftCents);
    impl->stretch.setTransposeSemitones(clampedCents / 100.0f);
}

void PitchShifter::setFormantBaseFrequency(float frequencyHz)
{
    if (frequencyHz > 0.0f && std::isfinite(frequencyHz))
        impl->stretch.setFormantBase(frequencyHz / static_cast<float>(sampleRate));
}

void PitchShifter::process(juce::AudioBuffer<float>& buffer)
{
    const int channelsToProcess = juce::jmin(buffer.getNumChannels(), preparedChannels);
    int bufferOffset = 0;

    while (bufferOffset < buffer.getNumSamples())
    {
        const int chunkSize = juce::jmin(preparedBlockSize, buffer.getNumSamples() - bufferOffset);

        for (int channel = 0; channel < channelsToProcess; ++channel)
            inputBuffer.copyFrom(channel, 0, buffer, channel, bufferOffset, chunkSize);

        for (int channel = channelsToProcess; channel < preparedChannels; ++channel)
            inputBuffer.clear(channel, 0, chunkSize);

        impl->stretch.process(inputBuffer.getArrayOfReadPointers(), chunkSize,
                              outputBuffer.getArrayOfWritePointers(), chunkSize);

        for (int channel = 0; channel < channelsToProcess; ++channel)
            buffer.copyFrom(channel, bufferOffset, outputBuffer, channel, 0, chunkSize);

        bufferOffset += chunkSize;
    }
}

int PitchShifter::getLatencySamples() const
{
    return latencySamples;
}
