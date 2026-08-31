#include "PluginEditor.h"
#include "Music/PitchConverter.h"

CrookedTuneAudioProcessorEditor::CrookedTuneAudioProcessorEditor(CrookedTuneAudioProcessor& processorToUse)
    : AudioProcessorEditor(&processorToUse), audioProcessor(processorToUse)
{
    setSize(600, 400);
    setResizable(true, true);
    setResizeLimits(500, 360, 1000, 700);

    pitchLabel.setText("No pitch detected", juce::dontSendNotification);
    pitchLabel.setJustificationType(juce::Justification::centred);
    pitchLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    pitchLabel.setFont(juce::Font(juce::FontOptions(18.0f)));
    addAndMakeVisible(pitchLabel);

    keyLabel.setText("Key", juce::dontSendNotification);
    keyLabel.setJustificationType(juce::Justification::centredRight);
    keyLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(keyLabel);

    keyBox.addItemList(
        juce::StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" },
        1);
    addAndMakeVisible(keyBox);
    keyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), "key", keyBox);

    scaleLabel.setText("Scale", juce::dontSendNotification);
    scaleLabel.setJustificationType(juce::Justification::centredRight);
    scaleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(scaleLabel);

    scaleBox.addItemList(juce::StringArray { "Chromatic", "Major", "Minor" }, 1);
    addAndMakeVisible(scaleBox);
    scaleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), "scale", scaleBox);

    strengthLabel.setText("Strength", juce::dontSendNotification);
    strengthLabel.setJustificationType(juce::Justification::centred);
    strengthLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(strengthLabel);

    strengthSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    strengthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    strengthSlider.setTextValueSuffix(" %");
    addAndMakeVisible(strengthSlider);
    strengthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "retuneStrength", strengthSlider);

    retuneLabel.setText("Retune", juce::dontSendNotification);
    retuneLabel.setJustificationType(juce::Justification::centred);
    retuneLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(retuneLabel);

    retuneSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    retuneSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    retuneSlider.setTextValueSuffix(" ms");
    addAndMakeVisible(retuneSlider);
    retuneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "retuneSpeed", retuneSlider);

    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.setJustificationType(juce::Justification::centred);
    mixLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(mixLabel);

    mixSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    mixSlider.setTextValueSuffix(" %");
    addAndMakeVisible(mixSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "mix", mixSlider);

    bypassButton.setButtonText("Bypass");
    bypassButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible(bypassButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "bypass", bypassButton);

    driveLabel.setText("Drive", juce::dontSendNotification);
    driveLabel.setJustificationType(juce::Justification::centred);
    driveLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(driveLabel);

    driveSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    driveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    driveSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(driveSlider);
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getValueTreeState(), "distortionDrive", driveSlider);

    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    gainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(gainLabel);

    gainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);

    addAndMakeVisible(gainSlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getValueTreeState(), "gain", gainSlider);

    startTimerHz(30);
}

CrookedTuneAudioProcessorEditor::~CrookedTuneAudioProcessorEditor()
{
    stopTimer();
}

void CrookedTuneAudioProcessorEditor::timerCallback()
{
    const float frequency = audioProcessor.getDetectedFrequency();
    const float confidence = audioProcessor.getPitchConfidence();
    const auto pitchInfo = PitchConverter::fromFrequency(frequency);
    const auto quantizedPitch = audioProcessor.getQuantizedPitch();
    const auto retuneState = audioProcessor.getRetuneState();

    if (pitchInfo.isValid && quantizedPitch.isValid && retuneState.isValid)
    {
        const auto centsPrefix = pitchInfo.cents >= 0.0f ? "+" : "";
        const auto correctionPrefix = quantizedPitch.correctionCents >= 0.0f ? "+" : "";
        const auto appliedPrefix = retuneState.appliedCorrectionCents >= 0.0f ? "+" : "";

        pitchLabel.setText("Detected: " + pitchInfo.noteName + " | "
                               + juce::String(pitchInfo.frequency, 1) + " Hz | "
                               + centsPrefix + juce::String(pitchInfo.cents, 1) + " cents\n"
                               + "Target: " + quantizedPitch.targetNoteName + " | Correction: "
                               + correctionPrefix + juce::String(quantizedPitch.correctionCents, 1) + " cents\n"
                               + "Applied: " + appliedPrefix
                               + juce::String(retuneState.appliedCorrectionCents, 1) + " cents | "
                               + "Confidence: "
                               + juce::String(juce::roundToInt(confidence * 100.0f)) + "%",
                           juce::dontSendNotification);
    }
    else
    {
        pitchLabel.setText("No pitch detected", juce::dontSendNotification);
    }
}

void CrookedTuneAudioProcessorEditor::paint(
    juce::Graphics& graphics)
{
    graphics.fillAll(juce::Colours::darkgrey);

    graphics.setColour(juce::Colours::white);

    graphics.setFont(28.0f);

    auto headerArea = getLocalBounds().removeFromTop(60);
    graphics.drawFittedText("CrookedTune", headerArea, juce::Justification::centred, 1);
}

void CrookedTuneAudioProcessorEditor::resized()
{
    auto availableArea = getLocalBounds().reduced(20);
    availableArea.removeFromTop(60);

    pitchLabel.setBounds(availableArea.removeFromTop(90));

    auto selectorsArea = availableArea.removeFromTop(35);
    auto bypassArea = selectorsArea.removeFromRight(90).reduced(5, 0);
    bypassButton.setBounds(bypassArea);
    auto keyArea = selectorsArea.removeFromLeft(selectorsArea.getWidth() / 2).reduced(5, 0);
    keyLabel.setBounds(keyArea.removeFromLeft(40));
    keyBox.setBounds(keyArea);

    auto scaleArea = selectorsArea.reduced(5, 0);
    scaleLabel.setBounds(scaleArea.removeFromLeft(50));
    scaleBox.setBounds(scaleArea);

    auto controlsArea = availableArea.removeFromBottom(145);
    const int controlWidth = controlsArea.getWidth() / 5;

    auto driveArea = controlsArea.removeFromLeft(controlWidth);
    driveLabel.setBounds(driveArea.removeFromTop(25));
    driveSlider.setBounds(driveArea);

    auto strengthArea = controlsArea.removeFromLeft(controlWidth);
    strengthLabel.setBounds(strengthArea.removeFromTop(25));
    strengthSlider.setBounds(strengthArea);

    auto retuneArea = controlsArea.removeFromLeft(controlWidth);
    retuneLabel.setBounds(retuneArea.removeFromTop(25));
    retuneSlider.setBounds(retuneArea);

    auto mixArea = controlsArea.removeFromLeft(controlWidth);
    mixLabel.setBounds(mixArea.removeFromTop(25));
    mixSlider.setBounds(mixArea);

    auto gainArea = controlsArea;
    gainLabel.setBounds(gainArea.removeFromTop(25));
    gainSlider.setBounds(gainArea);
}
