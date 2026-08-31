#pragma once

#include <JuceHeader.h>

class Distortion
{
public:
    Distortion() = default;

    void prepare(double sampleRate);
    void setDrive(float newDriveDb);
    void process(juce::AudioBuffer<float>& buffer);
    float getDrive() const;

private:
    static constexpr float maximumDriveDb = 24.0f;

    float targetDriveDb = 0.0f;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedDriveDb;
};
