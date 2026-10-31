#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class IronModularAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit IronModularAudioProcessorEditor(IronModularAudioProcessor&);
    ~IronModularAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    IronModularAudioProcessor& processor;
    juce::OwnedArray<juce::Slider> sliders;
    juce::OwnedArray<juce::Label> knobLabels, comboLabels;
    std::vector<std::unique_ptr<Attachment>> attachments;
    juce::OwnedArray<juce::ComboBox> combos;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAttachments;
    void addKnob(const juce::String& id, const juce::String& caption);
    void addChoice(const juce::String& id, const juce::String& caption);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IronModularAudioProcessorEditor)
};
