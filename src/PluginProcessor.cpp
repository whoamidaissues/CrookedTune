#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Music/PitchConverter.h"

#include <cmath>

CrookedTuneAudioProcessor::CrookedTuneAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
                      , valueTreeState(*this, nullptr, "Parameters", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout CrookedTuneAudioProcessor::createParameterLayout(){

    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    juce::NormalisableRange<float> gainRange(0.0f, 4.0f, 0.01f);
    layout.add(std::make_unique<juce::AudioParameterFloat>("gain", "Gain", gainRange, 0.5f));
    juce::NormalisableRange<float> driveRange(0.0f, 24.0f, 0.1f);
    layout.add(std::make_unique<juce::AudioParameterFloat>("distortionDrive", "Drive", driveRange, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "key", "Key",
        juce::StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" },
        0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "scale", "Scale",
        juce::StringArray { "Chromatic", "Major", "Minor" },
        0));
    juce::NormalisableRange<float> strengthRange(0.0f, 100.0f, 1.0f);
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "retuneStrength", "Strength", strengthRange, 100.0f));
    juce::NormalisableRange<float> retuneRange(0.0f, 200.0f, 1.0f);
    retuneRange.setSkewForCentre(30.0f);
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "retuneSpeed", "Retune Speed", retuneRange, 20.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "mix", "Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false));
    return layout;
}

juce::AudioProcessorValueTreeState& CrookedTuneAudioProcessor::getValueTreeState()
{
    return valueTreeState;
}

void CrookedTuneAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock){

    pitchDetector.prepare(sampleRate, samplesPerBlock);
    pitchShifter.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    dryWetMixer.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels(),
                        pitchShifter.getLatencySamples());
    retuneController.prepare(sampleRate);
    distortion.prepare(sampleRate);
    gain.prepare(sampleRate);
    previousRootNote = -1;
    previousScaleType = -1;
    setLatencySamples(pitchShifter.getLatencySamples());

    if (auto* driveParam = valueTreeState.getRawParameterValue("distortionDrive")){
        distortion.setDrive(driveParam->load());
    }
    if (auto* gainParam = valueTreeState.getRawParameterValue("gain")){
        gain.setGain(gainParam->load());
    }

}

float CrookedTuneAudioProcessor::getDetectedFrequency() const
{
    return pitchDetector.getDetectedFrequency();
}

float CrookedTuneAudioProcessor::getPitchConfidence() const
{
    return pitchDetector.getConfidence();
}

QuantizedPitch CrookedTuneAudioProcessor::getQuantizedPitch() const
{
    const auto retuneState = retuneController.getState();
    if (!retuneState.isValid)
        return {};

    QuantizedPitch result;
    result.targetMidiNote = retuneState.targetMidiNote;
    result.targetFrequency = 440.0f
        * std::pow(2.0f, (static_cast<float>(result.targetMidiNote) - 69.0f) / 12.0f);
    result.correctionCents = retuneState.rawCorrectionCents;
    result.correctionSemitones = result.correctionCents / 100.0f;
    result.targetNoteName = PitchConverter::fromFrequency(result.targetFrequency).noteName;
    result.isValid = true;
    return result;
}

RetuneState CrookedTuneAudioProcessor::getRetuneState() const
{
    return retuneController.getState();
}

void CrookedTuneAudioProcessor::releaseResources()
{
    pitchShifter.reset();
    dryWetMixer.reset();
}

bool CrookedTuneAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto inputLayout = layouts.getMainInputChannelSet();

    const auto outputLayout = layouts.getMainOutputChannelSet();

    const bool outputIsSupported = outputLayout == juce::AudioChannelSet::mono() || outputLayout == juce::AudioChannelSet::stereo();

    return outputIsSupported && inputLayout == outputLayout;
}


void CrookedTuneAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const int inputChannels = getTotalNumInputChannels();
    const int outputChannels = getTotalNumOutputChannels();


    for (int channel = inputChannels;channel < outputChannels;++channel)
    {
        buffer.clear(channel, 0, buffer.getNumSamples());
    }

    pitchDetector.process(buffer);
    dryWetMixer.captureDrySignal(buffer);

    const auto pitchInfo = PitchConverter::fromFrequency(pitchDetector.getDetectedFrequency());
    const auto* keyParam = valueTreeState.getRawParameterValue("key");
    const auto* scaleParam = valueTreeState.getRawParameterValue("scale");
    const auto* strengthParam = valueTreeState.getRawParameterValue("retuneStrength");
    const auto* retuneSpeedParam = valueTreeState.getRawParameterValue("retuneSpeed");

    if (pitchInfo.isValid && keyParam != nullptr && scaleParam != nullptr
        && strengthParam != nullptr && retuneSpeedParam != nullptr)
    {
        const int rootNote = juce::roundToInt(keyParam->load());
        const int scaleIndex = juce::roundToInt(scaleParam->load());
        const bool scaleChanged = rootNote != previousRootNote || scaleIndex != previousScaleType;
        const auto quantizedPitch = PitchQuantizer::quantize(
            pitchInfo.midiNote, rootNote, static_cast<ScaleType>(scaleIndex));

        retuneController.process(pitchInfo.midiNote,
                                 quantizedPitch,
                                 strengthParam->load(),
                                 retuneSpeedParam->load(),
                                 buffer.getNumSamples(),
                                 scaleChanged);
        previousRootNote = rootNote;
        previousScaleType = scaleIndex;
    }
    else
    {
        retuneController.process(0.0f, {}, 0.0f, 0.0f, buffer.getNumSamples(), false);
    }

    const auto retuneState = retuneController.getState();
    pitchShifter.setPitchShiftCents(
        retuneState.isValid ? retuneState.appliedCorrectionCents : 0.0f);
    pitchShifter.setFormantBaseFrequency(pitchDetector.getDetectedFrequency());
    pitchShifter.process(buffer);

    const auto* mixParam = valueTreeState.getRawParameterValue("mix");
    const auto* bypassParam = valueTreeState.getRawParameterValue("bypass");
    dryWetMixer.mixWithWetSignal(buffer,
                                 mixParam != nullptr ? mixParam->load() : 100.0f,
                                 bypassParam != nullptr && bypassParam->load() >= 0.5f);


    if (auto* gainParam = valueTreeState.getRawParameterValue("gain"))
    {
        gain.setGain(gainParam->load());
    }

    if (auto* driveParam = valueTreeState.getRawParameterValue("distortionDrive"))
    {
        distortion.setDrive(driveParam->load());
    }

    distortion.process(buffer);
    gain.process(buffer);

}

juce::AudioProcessorEditor*

CrookedTuneAudioProcessor::createEditor()
{
    return new CrookedTuneAudioProcessorEditor(*this);
}

bool CrookedTuneAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String

CrookedTuneAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CrookedTuneAudioProcessor::acceptsMidi() const
{
    return false;
}

bool CrookedTuneAudioProcessor::producesMidi() const
{
    return false;
}

bool CrookedTuneAudioProcessor::isMidiEffect() const
{
    return false;
}

double CrookedTuneAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CrookedTuneAudioProcessor::getNumPrograms()
{
    return 1;
}

int CrookedTuneAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CrookedTuneAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String

CrookedTuneAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);

    return {};
}

void CrookedTuneAudioProcessor::changeProgramName(int index,const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void CrookedTuneAudioProcessor::getStateInformation(juce::MemoryBlock& destinationData)
{
    auto state = valueTreeState.copyState();
    auto xmlState = state.createXml();

    copyXmlToBinary(*xmlState, destinationData);
}

void CrookedTuneAudioProcessor::setStateInformation(const void* data,int sizeInBytes)
{
    auto xmlState = getXmlFromBinary(data, sizeInBytes);

    if (xmlState != nullptr && xmlState->hasTagName(valueTreeState.state.getType()))
        valueTreeState.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor*

JUCE_CALLTYPE createPluginFilter()
{
    return new CrookedTuneAudioProcessor();
}
