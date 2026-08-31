#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CrookedTuneAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    explicit CrookedTuneAudioProcessorEditor(CrookedTuneAudioProcessor& processorToUse);

    ~CrookedTuneAudioProcessorEditor() override;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    void timerCallback() override;

    CrookedTuneAudioProcessor& audioProcessor;

    juce::Label pitchLabel;

    juce::Label keyLabel;
    juce::ComboBox keyBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> keyAttachment;

    juce::Label scaleLabel;
    juce::ComboBox scaleBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scaleAttachment;

    juce::Label strengthLabel;
    juce::Slider strengthSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> strengthAttachment;

    juce::Label retuneLabel;
    juce::Slider retuneSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> retuneAttachment;

    juce::Label mixLabel;
    juce::Slider mixSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    juce::ToggleButton bypassButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    juce::Label driveLabel;
    juce::Slider driveSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;

    juce::Label gainLabel;
    juce::Slider gainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CrookedTuneAudioProcessorEditor)
};
