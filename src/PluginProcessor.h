#pragma once

#include <JuceHeader.h>
#include "DSP/Distortion.h"
#include "DSP/DryWetMixer.h"
#include "DSP/Gain.h"
#include "DSP/PitchDetector.h"
#include "DSP/PitchShifter.h"
#include "Music/PitchQuantizer.h"
#include "Music/RetuneController.h"

class CrookedTuneAudioProcessor : public juce::AudioProcessor{
    public:

    CrookedTuneAudioProcessor();
    ~CrookedTuneAudioProcessor() override = default;

    
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;

    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;

    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;

    bool producesMidi() const override;

    bool isMidiEffect() const override;

    double getTailLengthSeconds() const override;

    int getNumPrograms() override;

    int getCurrentProgram() override;

    void setCurrentProgram(int index) override;

    const juce::String getProgramName(int index) override;

    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destinationData) override;

    void setStateInformation(const void* data,int sizeInBytes) override;
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState& getValueTreeState();

    float getDetectedFrequency() const;
    float getPitchConfidence() const;
    QuantizedPitch getQuantizedPitch() const;
    RetuneState getRetuneState() const;


    private:
    PitchDetector pitchDetector;
    PitchShifter pitchShifter;
    DryWetMixer dryWetMixer;
    RetuneController retuneController;
    Distortion distortion;
    Gain gain;

    int previousRootNote = -1;
    int previousScaleType = -1;
    
    juce::AudioProcessorValueTreeState valueTreeState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CrookedTuneAudioProcessor)
};
