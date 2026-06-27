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

    // Initialize TMT values
    tmtNode.driftBiasL = 1.0002f;
    tmtNode.driftBiasR = 0.9998f;
    tmtNode.internalToleranceScale = 1.001f;
    tmtNode.physicalCylinderID = 3;
}

SovereignArchonSuiteAudioProcessor::~SovereignArchonSuiteAudioProcessor() {}

void SovereignArchonSuiteAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    delayBuffer.assign(static_cast<int>(sampleRate * 3.0), 0.0f);
    reverbBuffer.assign(32768, 0.0f);
}

void SovereignArchonSuiteAudioProcessor::releaseResources() {}

bool SovereignArchonSuiteAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutput() == juce::AudioChannelSet::stereo();
}

void SovereignArchonSuiteAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);
    int numSamples = buffer.getNumSamples();

    const float A_Law_Constant = 87.6f;
    const float aLawDenominator = 1.0f + std::log(A_Law_Constant);
    const float lightMatrixConstant = 2.0f / 7.0f;
    double sampleRate = getSampleRate();

    for (int i = 0; i < numSamples; ++i)
    {
        float xL = leftChannel[i];
        float xR = rightChannel[i];

        float phaseAngle = (currentPhase % 72) * (2.0f * 3.14159265f / 72.0f);
        float engineModifier = std::sin(phaseAngle);

        float tmtLFO_L = std::sin(currentPhase * 0.00005f) * 0.001f;
        float tmtLFO_R = std::cos(currentPhase * 0.00004f) * 0.001f;
        float activeBiasL = tmtNode.driftBiasL + tmtLFO_L;
        float activeBiasR = tmtNode.driftBiasR + tmtLFO_R;

        switch (currentSelector)
        {
            case ARCHON_COMPRESSOR: {
                float signalEnergy = 0.5f * (std::abs(xL) + std::abs(xR));
                if (signalEnergy > 0.05f) 
                    fetGateVoltage = (0.991f * fetGateVoltage) + (0.009f * signalEnergy);
                else 
                    fetGateVoltage *= 0.9998f;

                float fetGainReduction = 1.0f / (1.0f + 5.0f * fetGateVoltage);
                xL *= fetGainReduction; xR *= fetGainReduction;
                xL = applyWhite72ALaw(xL, A_Law_Constant * activeBiasL, aLawDenominator);
                xR = applyWhite72ALaw(xR, A_Law_Constant * activeBiasR, aLawDenominator);
                break;
            }
            case ARCHON_SATURATION: {
                float drive = 1.35f;
                float distortedL = std::tanh(xL * drive * activeBiasL);
                float distortedR = std::tanh(xR * drive * activeBiasR);
                float valveL = std::pow(std::abs(xL + 0.05f), 1.5f) * (xL >= -0.05f ? 1.0f : -1.0f);
                float valveR = std::pow(std::abs(xR + 0.05f), 1.5f) * (xR >= -0.05f ? 1.0f : -1.0f);
                xL = (1.0f - lightMatrixConstant) * distortedL + lightMatrixConstant * valveL;
                xR = (1.0f - lightMatrixConstant) * distortedR + lightMatrixConstant * valveR;
                break;
            }
            case ARCHON_REVERB: {
                int plateSizeTap = 16381; 
                int readIdx = (reverbIndex - plateSizeTap + reverbBuffer.size()) % reverbBuffer.size();
                float modulationOffset = std::sin(currentPhase * 0.002f * activeBiasL) * 12.0f;
                int modulatedReadIdx = (readIdx + static_cast<int>(modulationOffset) + reverbBuffer.size()) % reverbBuffer.size();
                float acousticReflections = reverbBuffer[modulatedReadIdx];
                reverbBuffer[reverbIndex] = xL + (acousticReflections * (0.68f - lightMatrixConstant));
                reverbIndex = (reverbIndex + 1) % reverbBuffer.size();
                xL = (xL * 0.6f) + (acousticReflections * 0.4f);
                xR = (xR * 0.6f) + (acousticReflections * 0.4f);
                break;
            }
            case ARCHON_DELAY: {
                float flutterFactor = 1.0f + (0.003f * engineModifier * activeBiasL);
                int delayOffsetSamples = static_cast<int>(sampleRate * 0.5f * flutterFactor * (1.0f + lightMatrixConstant));
                int readIndex = (delayBufferIndex - delayOffsetSamples + delayBuffer.size()) % delayBuffer.size();
                float delayedSignal = delayBuffer[readIndex];
                delayBuffer[delayBufferIndex] = xL + (delayedSignal * 0.45f);
                delayBufferIndex = (delayBufferIndex + 1) % delayBuffer.size();
                xL += delayedSignal * 0.5f; xR += delayedSignal * 0.5f;
                break;
            }
            case ARCHON_EQUALIZER: {
                float pultecFilterAngle = 2.0f * 3.14159265f * 30.0f * activeBiasL * (static_cast<float>(i) / sampleRate);
                float filterWeight = std::sin(pultecFilterAngle);
                xL = xL + (xL * std::max(0.0f, filterWeight) * 0.15f) - (xL * std::max(0.0f, -filterWeight) * 0.12f);
                for (int cylinder = 1; cylinder <= 12; ++cylinder) {
                    if (cylinder % 2 == 0) xL = std::tanh(xL * (1.01f + 0.01f * engineModifier));
                    else xR *= (1.0f + 0.02f * std::cos(phaseAngle));
                }
                break;
            }
            case ARCHON_MAXIMIZER: {
                float crossoverAngle = 2.0f * 3.14159265f * 5000.0f * activeBiasR * (static_cast<float>(i) / sampleRate);
                float highPassWeight = std::abs(std::sin(crossoverAngle));
                float upperHighL = xL * highPassWeight; float upperHighR = xR * highPassWeight;
                xL += std::pow(upperHighL, 3.0f) * 0.22f * activeBiasL;
                xR += std::pow(upperHighR, 3.0f) * 0.22f * activeBiasR;
                break;
            }
        }

        currentPhase++;
        leftChannel[i]  = std::clamp(xL, -1.0f, 1.0f);
        rightChannel[i] = std::clamp(xR, -1.0f, 1.0f);
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
