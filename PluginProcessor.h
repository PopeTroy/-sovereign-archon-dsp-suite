#pragma once

#include <JuceHeader.h>
#include <vector>
#include <cmath>
#include <algorithm>

// =====================================================================
// GLOBAL SELECTOR CONSTANTS & STRUCTS
// =====================================================================
enum SovereignPluginSelector
{
    ARCHON_COMPRESSOR,
    ARCHON_SATURATION,
    ARCHON_REVERB,
    ARCHON_DELAY,
    ARCHON_EQUALIZER,
    ARCHON_MAXIMIZER
};

struct TMTProfile
{
    float driftBiasL;
    float driftBiasR;
    float internalToleranceScale;
    int physicalCylinderID;
};

// =====================================================================
// PRIMARY AUDIO PROCESSOR CLASS DECLARATION
// =====================================================================
class SovereignArchonSuiteAudioProcessor  : public juce::AudioProcessor
{
public:
    SovereignArchonSuiteAudioProcessor();
    ~SovereignArchonSuiteAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Sovereign Archon Suite"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int index) override {}
    const juce::String getProgramName (int index) override { return {}; }
    void changeProgramName (int index, const juce::String& newName) override {}

    void getStateInformation (juce::MemoryBlock& destData) override {}
    void setStateInformation (const void* data, int sizeInBytes) override {}

    // Runtime Selection Interfaces
    SovereignPluginSelector getActiveMode() const { return currentSelector; }
    void setActiveMode(SovereignPluginSelector newMode) { currentSelector = newMode; }

    // SUCCESS FIX: Explicitly declared the UI engine tracking function
    float getGainReductionValue() const;

private:
    SovereignPluginSelector currentSelector;
    unsigned long long currentPhase;
    TMTProfile tmtNode;

    // Parameter References
    juce::AudioParameterFloat* thresholdParam;
    juce::AudioParameterFloat* ratioParam;
    juce::AudioParameterFloat* mixParam;

    // Linear Crossfading Envelopes for Anti-Zipper Operations
    juce::LinearSmoothedValue<float> smoothedThreshold;
    juce::LinearSmoothedValue<float> smoothedRatio;
    juce::LinearSmoothedValue<float> smoothedMix;

    // Core Analog Component Variables
    float fetGateVoltage;
    
    // SUCCESS FIX: Unified Field State Space & Power Rail Sag Variables Declared
    float powerRailSag;
    float stateSpaceMemoryL;
    float stateSpaceMemoryR;
    
    // Isolated Left and Right Memory Tracking Vectors
    std::vector<float> delayBufferL;
    std::vector<float> delayBufferR;
    int delayWriteIndex;
    
    std::vector<float> reverbBufferL;
    std::vector<float> reverbBufferR;
    int reverbWriteIndex;

    float applyWhite72ALaw (float sample, float A, float denominator);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SovereignArchonSuiteAudioProcessor)
};
