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

    // Upgraded Dedicated Hardware Control Array
    juce::Slider inputGainKnob;
    juce::Slider param1Knob; // Multi-purpose: Threshold, Drive, Plate Size, etc.
    juce::Slider param2Knob; // Multi-purpose: Ratio, Valve Warmth, Feedback
    juce::Slider outputTrimKnob;
    juce::Slider tmtChannelKnob;
    juce::Slider stereoWidthKnob;

    juce::TextButton compressorModeBtn { "COMP" }, saturationModeBtn { "SAT" }, 
                     reverbModeBtn { "REVERB" }, delayModeBtn { "DELAY" }, 
                     eqModeBtn { "EQ" }, maximizerModeBtn { "MAX" };

    SovereignArchonSuiteAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SovereignArchonSuiteAudioProcessorEditor)
};
