#include "Distortion.h"

#include <cmath>

void Distortion::prepare(double sampleRate)
{
    constexpr double smoothingTimeSeconds = 0.02;

    smoothedDriveDb.reset(sampleRate, smoothingTimeSeconds);
    smoothedDriveDb.setCurrentAndTargetValue(targetDriveDb);
}

void Distortion::setDrive(float newDriveDb)
{
    targetDriveDb = juce::jlimit(0.0f, maximumDriveDb, newDriveDb);
    smoothedDriveDb.setTargetValue(targetDriveDb);
}

void Distortion::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const float driveDb = smoothedDriveDb.getNextValue();
        const float driveGain = juce::Decibels::decibelsToGain(driveDb);
        const float amount = driveDb / maximumDriveDb;

        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* samples = buffer.getWritePointer(channel);
            const float cleanSample = samples[sample];
            const float distortedSample = std::tanh(cleanSample * driveGain);

            samples[sample] = cleanSample + amount * (distortedSample - cleanSample);
        }
    }
}

float Distortion::getDrive() const
{
    return targetDriveDb;
}
