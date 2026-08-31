#pragma once

#include <JuceHeader.h>

#include <memory>

class PitchShifter
{
public:
    PitchShifter();
    ~PitchShifter();

    void prepare(double newSampleRate, int maximumBlockSize, int numChannels);
    void reset();
    void setPitchShiftCents(float newPitchShiftCents);
    void setFormantBaseFrequency(float frequencyHz);
    void process(juce::AudioBuffer<float>& buffer);

    int getLatencySamples() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl;

    juce::AudioBuffer<float> inputBuffer;
    juce::AudioBuffer<float> outputBuffer;

    double sampleRate = 44100.0;
    int preparedChannels = 0;
    int preparedBlockSize = 0;
    int latencySamples = 0;
};
