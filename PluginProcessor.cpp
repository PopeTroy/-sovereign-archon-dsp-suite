#include "PluginProcessor.h"
#include "PluginEditor.h"

SovereignArchonSuiteAudioProcessor::SovereignArchonSuiteAudioProcessor()
    : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      currentSelector(ARCHON_COMPRESSOR), currentPhase(0), fetGateVoltage(0.0f), delayBufferIndex(0), reverbIndex(0)
{
    addParameter (thresholdParam = new juce::AudioParameterFloat ("threshold", "Threshold", -60.0f, 0.0f, -14.0f));
    addParameter (ratioParam     = new juce::AudioParameterFloat ("ratio", "Ratio", 1.0f, 20.0f, 4.0f));
    addParameter (mixParam       = new juce::AudioParameterFloat ("mix", "Mix", 0.0f, 100.0f, 100.0f));

    tmtNode.driftBiasL = 1.0002f;
    tmtNode.driftBiasR = 0.9998f;
    tmtNode.internalToleranceScale = 1.001f;
    tmtNode.physicalCylinderID = 3;
}

SovereignArchonSuiteAudioProcessor::~SovereignArchonSuiteAudioProcessor() {}

void SovereignArchonSuiteAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    delayBuffer.assign(static_cast<int>(sampleRate * 3.0), 0.0f);
    reverbBuffer.assign(65536, 0.0f); // Doubled and padded memory boundary
    delayBufferIndex = 0;
    reverbIndex = 0;
}

void SovereignArchonSuiteAudioProcessor::releaseResources() {}

bool SovereignArchonSuiteAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void SovereignArchonSuiteAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (totalNumInputChannels < 2) return;

    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);
    int numSamples = buffer.getNumSamples();

    const float A_Law_Constant = 87.6f;
    const float aLawDenominator = 1.0f + std::log(A_Law_Constant);
    const float lightMatrixConstant = 2.0f / 7.0f;
    double sampleRate = getSampleRate();

    // SUCCESS FIX: Evaluate the switch outside the sample loops to eliminate CPU branch stalling
    switch (currentSelector)
    {
        case ARCHON_COMPRESSOR:
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float xL = leftChannel[i];
                float xR = rightChannel[i];
                float signalEnergy = 0.5f * (std::abs(xL) + std::abs(xR));
                
                fetGateVoltage = (signalEnergy > 0.05f) ? ((0.991f * fetGateVoltage) + (0.009f * signalEnergy)) : (fetGateVoltage * 0.9998f);
                float fetGainReduction = 1.0f / (1.0f + 5.0f * fetGateVoltage);
                
                xL *= fetGainReduction; 
                xR *= fetGainReduction;

                float tmtLFO = std::sin(currentPhase * 0.00005f) * 0.001f;
                xL = applyWhite72ALaw(xL, A_Law_Constant * (tmtNode.driftBiasL + tmtLFO), aLawDenominator);
                xR = applyWhite72ALaw(xR, A_Law_Constant * (tmtNode.driftBiasR + tmtLFO), aLawDenominator);
                
                leftChannel[i]  = std::clamp(xL, -1.0f, 1.0f);
                rightChannel[i] = std::clamp(xR, -1.0f, 1.0f);
                currentPhase++;
            }
            break;
        }

        case ARCHON_SATURATION:
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float tmtLFO = std::sin(currentPhase * 0.00005f) * 0.001f;
                float drive = 1.35f;
                
                float distortedL = std::tanh(leftChannel[i] * drive * (tmtNode.driftBiasL + tmtLFO));
                float distortedR = std::tanh(rightChannel[i] * drive * (tmtNode.driftBiasR + tmtLFO));
                
                float valveL = std::pow(std::abs(leftChannel[i] + 0.05f), 1.5f) * (leftChannel[i] >= -0.05f ? 1.0f : -1.0f);
                float valveR = std::pow(std::abs(rightChannel[i] + 0.05f), 1.5f) * (rightChannel[i] >= -0.05f ? 1.0f : -1.0f);
                
                leftChannel[i]  = std::clamp((1.0f - lightMatrixConstant) * distortedL + lightMatrixConstant * valveL, -1.0f, 1.0f);
                rightChannel[i] = std::clamp((1.0f - lightMatrixConstant) * distortedR + lightMatrixConstant * valveR, -1.0f, 1.0f);
                currentPhase++;
            }
            break;
        }

        case ARCHON_REVERB:
        {
            int bufSize = static_cast<int>(reverbBuffer.size());
            for (int i = 0; i < numSamples; ++i)
            {
                float tmtLFO = std::sin(currentPhase * 0.00005f) * 0.001f;
                int plateSizeTap = 16381; 
                
                int readIdx = (reverbIndex - plateSizeTap + bufSize) % bufSize;
                float modulationOffset = std::sin(currentPhase * 0.002f * (tmtNode.driftBiasL + tmtLFO)) * 12.0f;
                int modulatedReadIdx = (readIdx + static_cast<int>(modulationOffset) + bufSize) % bufSize;
                
                float acousticReflections = reverbBuffer[modulatedReadIdx];
                reverbBuffer[reverbIndex] = leftChannel[i] + (acousticReflections * (0.68f - lightMatrixConstant));
                reverbIndex = (reverbIndex + 1) % bufSize;
                
                leftChannel[i]  = std::clamp((leftChannel[i] * 0.6f) + (acousticReflections * 0.4f), -1.0f, 1.0f);
                rightChannel[i] = std::clamp((rightChannel[i] * 0.6f) + (acousticReflections * 0.4f), -1.0f, 1.0f);
                currentPhase++;
            }
            break;
        }

        case ARCHON_DELAY:
        {
            int dBufSize = static_cast<int>(delayBuffer.size());
            for (int i = 0; i < numSamples; ++i)
            {
                float phaseAngle = (currentPhase % 72) * (2.0f * 3.14159265f / 72.0f);
                float tmtLFO = std::sin(currentPhase * 0.00005f) * 0.001f;
                
                float flutterFactor = 1.0f + (0.003f * std::sin(phaseAngle) * (tmtNode.driftBiasL + tmtLFO));
                int delayOffsetSamples = static_cast<int>(sampleRate * 0.5f * flutterFactor * (1.0f + lightMatrixConstant));
                
                int readIndex = (delayBufferIndex - delayOffsetSamples + dBufSize) % dBufSize;
                float delayedSignal = delayBuffer[readIndex];
                
                delayBuffer[delayBufferIndex] = leftChannel[i] + (delayedSignal * 0.45f);
                delayBufferIndex = (delayBufferIndex + 1) % dBufSize;
                
                leftChannel[i]  = std::clamp(leftChannel[i] + (delayedSignal * 0.5f), -1.0f, 1.0f);
                rightChannel[i] = std::clamp(rightChannel[i] + (delayedSignal * 0.5f), -1.0f, 1.0f);
                currentPhase++;
            }
            break;
        }

        case ARCHON_EQUALIZER:
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float phaseAngle = (currentPhase % 72) * (2.0f * 3.14159265f / 72.0f);
                float tmtLFO = std::sin(currentPhase * 0.00005f) * 0.001f;
                
                float pultecFilterAngle = 2.0f * 3.14159265f * 30.0f * (tmtNode.driftBiasL + tmtLFO) * (static_cast<float>(i) / sampleRate);
                float filterWeight = std::sin(pultecFilterAngle);
                
                float xL = leftChannel[i] + (leftChannel[i] * std::max(0.0f, filterWeight) * 0.15f) - (leftChannel[i] * std::max(0.0f, -filterWeight) * 0.12f);
                float xR = rightChannel[i];

                float engineModifier = std::sin(phaseAngle);
                for (int cylinder = 1; cylinder <= 12; ++cylinder) {
                    if (cylinder % 2 == 0) xL = std::tanh(xL * (1.01f + 0.01f * engineModifier));
                    else xR *= (1.0f + 0.02f * std::cos(phaseAngle));
                }
                
                leftChannel[i]  = std::clamp(xL, -1.0f, 1.0f);
                rightChannel[i] = std::clamp(xR, -1.0f, 1.0f);
                currentPhase++;
            }
            break;
        }

        case ARCHON_MAXIMIZER:
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float tmtLFO = std::sin(currentPhase * 0.00005f) * 0.001f;
                float crossoverAngle = 2.0f * 3.14159265f * 5000.0f * (tmtNode.driftBiasR + tmtLFO) * (static_cast<float>(i) / sampleRate);
                float highPassWeight = std::abs(std::sin(crossoverAngle));
                
                float upperHighL = leftChannel[i] * highPassWeight; 
                float upperHighR = rightChannel[i] * highPassWeight;
                
                float xL = leftChannel[i] + (std::pow(upperHighL, 3.0f) * 0.22f * (tmtNode.driftBiasL + tmtLFO));
                float xR = rightChannel[i] + (std::pow(upperHighR, 3.0f) * 0.22f * (tmtNode.driftBiasR + tmtLFO));
                
                leftChannel[i]  = std::clamp(xL, -1.0f, 1.0f);
                rightChannel[i] = std::clamp(xR, -1.0f, 1.0f);
                currentPhase++;
            }
            break;
        }
    }
}

float SovereignArchonSuiteAudioProcessor::applyWhite72ALaw(float sample, float A, float denominator)
{
    float absSample = std::abs(sample);
    float sign = (sample > 0.0f) ? 1.0f : -1.0f;
    if (absSample <= (1.0f / A)) return sign * ((A * absSample) / denominator);
    return sign * ((1.0f + std::log(A * absSample)) / denominator);
}

juce::AudioProcessorEditor* SovereignArchonSuiteAudioProcessor::createEditor()
{
    return new SovereignArchonSuiteAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SovereignArchonSuiteAudioProcessor();
}
