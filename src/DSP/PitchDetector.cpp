#include "PitchDetector.h"

#include <cmath>

void PitchDetector::prepare(double newSampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);

    sampleRate = newSampleRate;
    ringBuffer.fill(0.0f);
    analysisBuffer.fill(0.0f);
    differenceBuffer.fill(0.0f);
    writePosition = 0;
    samplesInBuffer = 0;
    samplesSinceLastAnalysis = 0;
    lastAcceptedFrequency = 0.0f;
    pendingOctaveFrequency = 0.0f;
    pendingOctaveCount = 0;
    detectedFrequency.store(0.0f);
    confidence.store(0.0f);
}

void PitchDetector::process(const juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();

    if (numChannels == 0)
        return;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float monoSample = 0.0f;

        for (int channel = 0; channel < numChannels; ++channel)
            monoSample += buffer.getReadPointer(channel)[sample];

        monoSample /= static_cast<float>(numChannels);
        ringBuffer[static_cast<std::size_t>(writePosition)] = monoSample;
        writePosition = (writePosition + 1) % analysisWindowSize;
        samplesInBuffer = juce::jmin(samplesInBuffer + 1, analysisWindowSize);
        ++samplesSinceLastAnalysis;

        if (samplesInBuffer == analysisWindowSize
            && samplesSinceLastAnalysis >= analysisHopSize)
        {
            samplesSinceLastAnalysis = 0;
            analyseWindow();
        }
    }
}

float PitchDetector::getDetectedFrequency() const
{
    return detectedFrequency.load();
}

float PitchDetector::getConfidence() const
{
    return confidence.load();
}

void PitchDetector::analyseWindow()
{
    float mean = 0.0f;

    for (int index = 0; index < analysisWindowSize; ++index)
    {
        const int ringIndex = (writePosition + index) % analysisWindowSize;
        analysisBuffer[static_cast<std::size_t>(index)] = ringBuffer[static_cast<std::size_t>(ringIndex)];
        mean += analysisBuffer[static_cast<std::size_t>(index)];
    }

    mean /= static_cast<float>(analysisWindowSize);

    float sumOfSquares = 0.0f;
    for (auto& sample : analysisBuffer)
    {
        sample -= mean;
        sumOfSquares += sample * sample;
    }

    const float rms = std::sqrt(sumOfSquares / static_cast<float>(analysisWindowSize));
    if (rms < minimumRms)
    {
        lastAcceptedFrequency = 0.0f;
        pendingOctaveFrequency = 0.0f;
        pendingOctaveCount = 0;
        publishResult(0.0f, 0.0f);
        return;
    }

    const int minimumLag = juce::jmax(2, static_cast<int>(std::floor(sampleRate / maximumFrequency)));
    const int maximumLag = juce::jmin(analysisWindowSize - 2,
                                      static_cast<int>(std::ceil(sampleRate / minimumFrequency)));
    const int comparisonLength = analysisWindowSize - maximumLag;

    differenceBuffer.fill(0.0f);

    for (int lag = 1; lag <= maximumLag; ++lag)
    {
        float difference = 0.0f;

        for (int index = 0; index < comparisonLength; ++index)
        {
            const float delta = analysisBuffer[static_cast<std::size_t>(index)]
                              - analysisBuffer[static_cast<std::size_t>(index + lag)];
            difference += delta * delta;
        }

        differenceBuffer[static_cast<std::size_t>(lag)] = difference;
    }

    float runningSum = 0.0f;
    differenceBuffer[0] = 1.0f;

    for (int lag = 1; lag <= maximumLag; ++lag)
    {
        runningSum += differenceBuffer[static_cast<std::size_t>(lag)];
        differenceBuffer[static_cast<std::size_t>(lag)] = runningSum > 0.0f
            ? differenceBuffer[static_cast<std::size_t>(lag)] * static_cast<float>(lag) / runningSum
            : 1.0f;
    }

    int bestLag = 0;
    for (int lag = minimumLag; lag <= maximumLag; ++lag)
    {
        if (differenceBuffer[static_cast<std::size_t>(lag)] < yinThreshold)
        {
            while (lag + 1 <= maximumLag
                   && differenceBuffer[static_cast<std::size_t>(lag + 1)]
                        < differenceBuffer[static_cast<std::size_t>(lag)])
            {
                ++lag;
            }

            bestLag = lag;
            break;
        }
    }

    if (bestLag == 0)
    {
        publishResult(0.0f, 0.0f);
        return;
    }

    float refinedLag = static_cast<float>(bestLag);
    if (bestLag > minimumLag && bestLag < maximumLag)
    {
        const float left = differenceBuffer[static_cast<std::size_t>(bestLag - 1)];
        const float centre = differenceBuffer[static_cast<std::size_t>(bestLag)];
        const float right = differenceBuffer[static_cast<std::size_t>(bestLag + 1)];
        const float denominator = left - 2.0f * centre + right;

        if (std::abs(denominator) > 1.0e-9f)
            refinedLag += 0.5f * (left - right) / denominator;
    }

    const float candidateFrequency = static_cast<float>(sampleRate) / refinedLag;
    const float candidateConfidence = juce::jlimit(0.0f, 1.0f,
        1.0f - differenceBuffer[static_cast<std::size_t>(bestLag)]);

    if (lastAcceptedFrequency > 0.0f)
    {
        const float ratio = candidateFrequency / lastAcceptedFrequency;
        const bool looksLikeOctaveJump = (ratio > 1.85f && ratio < 2.15f)
                                      || (ratio > 0.46f && ratio < 0.54f);

        if (looksLikeOctaveJump)
        {
            const bool matchesPending = pendingOctaveFrequency > 0.0f
                && std::abs(candidateFrequency - pendingOctaveFrequency) / pendingOctaveFrequency < 0.05f;

            pendingOctaveFrequency = candidateFrequency;
            pendingOctaveCount = matchesPending ? pendingOctaveCount + 1 : 1;

            if (pendingOctaveCount < 3)
            {
                publishResult(lastAcceptedFrequency, candidateConfidence);
                return;
            }
        }
    }

    pendingOctaveFrequency = 0.0f;
    pendingOctaveCount = 0;

    const float smoothedFrequency = lastAcceptedFrequency > 0.0f
        ? 0.75f * lastAcceptedFrequency + 0.25f * candidateFrequency
        : candidateFrequency;

    lastAcceptedFrequency = smoothedFrequency;
    publishResult(smoothedFrequency, candidateConfidence);
}

void PitchDetector::publishResult(float frequencyToPublish, float confidenceToPublish)
{
    detectedFrequency.store(frequencyToPublish);
    confidence.store(confidenceToPublish);
}
