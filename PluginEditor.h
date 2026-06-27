#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class SovereignArchonSuiteAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                                  private juce::Timer
{
public:
    SovereignArchonSuiteAudioProcessorEditor (SovereignArchonSuiteAudioProcessor&);
    ~SovereignArchonSuiteAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    juce::Slider thresholdKnob;
    juce::Slider ratioKnob;
    juce::Slider mixKnob;

    SovereignArchonSuiteAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SovereignArchonSuiteAudioProcessorEditor)
};
