#include "PluginProcessor.h"
#include "PluginEditor.h"

SovereignArchonSuiteAudioProcessor::SovereignArchonSuiteAudioProcessor()
    : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      currentSelector(ARCHON_COMPRESSOR), currentPhase(0), fetGateVoltage(0.0f)
{
    // Register parameters explicitly to link into our automated GUI framework layout
    addParameter (thresholdParam = new juce::AudioParameterFloat ("threshold", "Threshold", -60.0f, 0.0f, -14.0f));
    addParameter (ratioParam     = new juce::AudioParameterFloat ("ratio", "Ratio", 1.0f, 20.0f, 4.0f));
    addParameter (mixParam       = new juce::AudioParameterFloat ("mix", "Mix", 0.0f, 100.0f, 100.0f));

    tmtNode.driftBiasL = 1.0002f;
    tmtNode.driftBiasR = 0.9998f;
    tmtNode.internalToleranceScale = 1.001f;
    tmtNode.physicalCylinderID = 3;

    // Smoother coefficients initialization
    smoothedThreshold.setCurrentAndTargetValue (-14.0f);
    smoothedRatio.setCurrentAndTargetValue (4.0f);
    smoothedMix.setCurrentAndTargetValue (100.0f);
}

SovereignArchonSuiteAudioProcessor::~SovereignArchonSuiteAudioProcessor() {}

void SovereignArchonSuiteAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // SUCCESS FIX: Separate delay and reverb buffers into discrete Left and Right channels
    const int maxDelaySamples = static_cast<int>(sampleRate * 3.0);
    delayBufferL.assign (maxDelaySamples, 0.0f);
    delayBufferR.assign (maxDelaySamples, 0.0f);
    
    reverbBufferL.assign (65536, 0.0f);
    reverbBufferR.assign (65536, 0.0f);

    delayWriteIndex = 0;
    reverbWriteIndex = 0;

    smoothedThreshold.reset (sampleRate, 0.02); // 20ms smoothing ramp
    smoothedRatio.reset (sampleRate, 0.02);
    smoothedMix.reset (sampleRate, 0.02);
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
    const float aLawDenominator = 1.0f + std::log (A_Law_Constant);
    const float lightMatrixConstant = 2.0f / 7.0f;
    double sampleRate = getSampleRate();

    // Push targets to interpolation ramps to stop slider click artifacts completely
    smoothedThreshold.setTargetValue (thresholdParam->get());
    smoothedRatio.setTargetValue (ratioParam->get());
    smoothedMix.setTargetValue (mixParam->get());

    switch (currentSelector)
    {
        case ARCHON_COMPRESSOR:
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float targetDb = smoothedThreshold.getNextValue();
                float targetRatio = smoothedRatio.getNextValue();
                float targetThreshLinear = juce::Decibels::decibelsToGain (targetDb);

                float xL = leftChannel[i];
                float xR = rightChannel[i];
                float signalEnergy = 0.5f * (std::abs (xL) + std::abs (xR));
                
                // Track signal attack envelope against interactive target threshold parameter limits
                if (signalEnergy > targetThreshLinear)
                    fetGateVoltage = (0.95f * fetGateVoltage) + (0.05f * (signalEnergy - targetThreshLinear));
                else
                    fetGateVoltage *= 0.999f;

                float crunchModifier = (targetRatio > 1.0f) ? (targetRatio - 1.0f) * 0.5f : 0.0f;
                float fetGainReduction = 1.0f / (1.0f + (5.0f + crunchModifier) * fetGateVoltage);
                
                xL *= fetGainReduction; 
                xR *= fetGainReduction;

                float tmtLFO = std::sin (currentPhase * 0.00005f) * 0.001f;
                xL = applyWhite72ALaw (xL, A_Law_Constant * (tmtNode.driftBiasL + tmtLFO), aLawDenominator);
                xR = applyWhite72ALaw (xR, A_Law_Constant * (tmtNode.driftBiasR + tmtLFO), aLawDenominator);
                
                leftChannel[i]  = std::clamp (xL, -1.0f, 1.0f);
                rightChannel[i] = std::clamp (xR, -1.0f, 1.0f);
                currentPhase++;
            }
            break;
        }

        case ARCHON_SATURATION:
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float ratioDriveAdjust = (smoothedRatio.getNextValue() / 20.0f) * 0.5f;
                float drive = 1.35f + ratioDriveAdjust;
                float mix = smoothedMix.getNextValue() / 100.0f;

                float tmtLFO = std::sin (currentPhase * 0.00005f) * 0.001f;
                
                float origL = leftChannel[i];
                float origRefR = rightChannel[i];

                float distortedL = std::tanh (origL * drive * (tmtNode.driftBiasL + tmtLFO));
                float distortedR = std::tanh (origRefR * drive * (tmtNode.driftBiasR + tmtLFO));
                
                float valveL = std::pow (std::abs (origL + 0.05f), 1.5f) * (origL >= -0.05f ? 1.0f : -1.0f);
                float valveR = std::pow (std::abs (origRefR + 0.05f), 1.5f) * (origRefR >= -0.05f ? 1.0f : -1.0f);
                
                float processL = (1.0f - lightMatrixConstant) * distortedL + lightMatrixConstant * valveL;
                float processR = (1.0f - lightMatrixConstant) * distortedR + lightMatrixConstant * valveR;

                leftChannel[i]  = std::clamp ((1.0f - mix) * origL + mix * processL, -1.0f, 1.0f);
                rightChannel[i] = std::clamp ((1.0f - mix) * origRefR + mix * processR, -1.0f, 1.0f);
                currentPhase++;
            }
            break;
        }

        case ARCHON_REVERB:
        {
            int bufSize = static_cast<int>(reverbBufferL.size());
            float mix = smoothedMix.getNextValue() / 100.0f;

            for (int i = 0; i < numSamples; ++i)
            {
                float tmtLFO = std::sin (currentPhase * 0.00005f) * 0.001f;
                int plateSizeTap = 16381 + static_cast<int>((smoothedRatio.getNextValue() / 20.0f) * 4000.0f); 
                
                int readIdx = (reverbWriteIndex - plateSizeTap + bufSize) % bufSize;
                
                // Left Space Pathing Array
                float modulationOffsetL = std::sin (currentPhase * 0.002f * (tmtNode.driftBiasL + tmtLFO)) * 12.0f;
                int modulatedReadIdxL = (readIdx + static_cast<int>(modulationOffsetL) + bufSize) % bufSize;
                float acousticReflectionsL = reverbBufferL[modulatedReadIdxL];
                reverbBufferL[reverbWriteIndex] = leftChannel[i] + (acousticReflectionsL * (0.68f - lightMatrixConstant));

                // Right Space Pathing Array (Isolated mapping completely skips internal channel bleed clicks)
                float modulationOffsetR = std::cos (currentPhase * 0.0018f * (tmtNode.driftBiasR + tmtLFO)) * 11.0f;
                int modulatedReadIdxR = (readIdx + static_cast<int>(modulationOffsetR) + bufSize) % bufSize;
                float acousticReflectionsR = reverbBufferR[modulatedReadIdxR];
                reverbBufferR[reverbWriteIndex] = rightChannel[i] + (acousticReflectionsR * (0.68f - lightMatrixConstant));

                reverbWriteIndex = (reverbWriteIndex + 1) % bufSize;
                
                leftChannel[i]  = std::clamp ((1.0f - mix) * leftChannel[i] + mix * ((leftChannel[i] * 0.5f) + (acousticReflectionsL * 0.5f)), -1.0f, 1.0f);
                rightChannel[i] = std::clamp ((1.0f - mix) * rightChannel[i] + mix * ((rightChannel[i] * 0.5f) + (acousticReflectionsR * 0.5f)), -1.0f, 1.0f);
                currentPhase++;
            }
            break;
        }

        case ARCHON_DELAY:
        {
            int dBufSize = static_cast<int>(delayBufferL.size());
            float mix = smoothedMix.getNextValue() / 100.0f;

            for (int i = 0; i < numSamples; ++i)
            {
                float phaseAngle = (currentPhase % 72) * (2.0f * 3.14159265f / 72.0f);
                float tmtLFO = std::sin (currentPhase * 0.00005f) * 0.001f;
                
                float flutterFactor = 1.0f + (0.003f * std::sin (phaseAngle) * (tmtNode.driftBiasL + tmtLFO));
                int delayOffsetSamples = static_cast<int>(sampleRate * 0.5f * flutterFactor * (1.0f + lightMatrixConstant));
                
                int readIndex = (delayWriteIndex - delayOffsetSamples + dBufSize) % dBufSize;
                
                float delayedSignalL = delayBufferL[readIndex];
                float delayedSignalR = delayBufferR[readIndex];
                
                delayBufferL[delayWriteIndex] = leftChannel[i] + (delayedSignalL * 0.45f);
                delayBufferR[delayWriteIndex] = rightChannel[i] + (delayedSignalR * 0.45f);
                delayWriteIndex = (delayWriteIndex + 1) % dBufSize;
                
                leftChannel[i]  = std::clamp ((1.0f - mix) * leftChannel[i] + mix * (leftChannel[i] + (delayedSignalL * 0.5f)), -1.0f, 1.0f);
                rightChannel[i] = std::clamp ((1.0f - mix) * rightChannel[i] + mix * (rightChannel[i] + (delayedSignalR * 0.5f)), -1.0f, 1.0f);
                currentPhase++;
            }
            break;
        }

        case ARCHON_EQUALIZER:
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float phaseAngle = (currentPhase % 72) * (2.0f * 3.14159265f / 72.0f);
                float tmtLFO = std::sin (currentPhase * 0.00005f) * 0.001f;
                
                float pultecFilterAngle = 2.0f * 3.14159265f * 30.0f * (tmtNode.driftBiasL + tmtLFO) * (static_cast<float>(i) / sampleRate);
                float filterWeight = std::sin (pultecFilterAngle);
                
                float eqGainFactor = (smoothedThreshold.getNextValue() + 60.0f) / 60.0f * 0.2f; 
                float xL = leftChannel[i] + (leftChannel[i] * std::max (0.0f, filterWeight) * eqGainFactor) - (leftChannel[i] * std::max (0.0f, -filterWeight) * 0.12f);
                float xR = rightChannel[i];

                float engineModifier = std::sin (phaseAngle);
                for (int cylinder = 1; cylinder <= 12; ++cylinder) {
                    if (cylinder % 2 == 0) xL = std::tanh (xL * (1.01f + 0.01f * engineModifier));
                    else xR *= (1.0f + 0.02f * std::cos (phaseAngle));
                }
                
                leftChannel[i]  = std::clamp (xL, -1.0f, 1.0f);
                rightChannel[i] = std::clamp (xR, -1.0f, 1.0f);
                currentPhase++;
            }
            break;
        }

        case ARCHON_MAXIMIZER:
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float tmtLFO = std::sin (currentPhase * 0.00005f) * 0.001f;
                float threshDb = smoothedThreshold.getNextValue();
                float driveScalar = juce::Decibels::decibelsToGain (-threshDb * 0.5f);

                float crossoverAngle = 2.0f * 3.14159265f * 5000.0f * (tmtNode.driftBiasR + tmtLFO) * (static_cast<float>(i) / sampleRate);
                float highPassWeight = std::abs (std::sin (crossoverAngle));
                
                float inputL = leftChannel[i] * driveScalar;
                float inputR = rightChannel[i] * driveScalar;

                float upperHighL = inputL * highPassWeight; 
                float upperHighR = inputR * highPassWeight;
                
                // Smooth anti-aliased saturation curves prevent harmonic folding artifacts
                float processL = inputL + (std::pow (upperHighL, 3.0f) * 0.15f * (tmtNode.driftBiasL + tmtLFO));
                float processR = inputR + (std::pow (upperHighR, 3.0f) * 0.15f * (tmtNode.driftBiasR + tmtLFO));
                
                // Brickwall dynamic limiting clamp
                leftChannel[i]  = std::clamp (processL, -0.98f, 0.98f);
                rightChannel[i] = std::clamp (processR, -0.98f, 0.98f);
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
