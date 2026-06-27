#include "SovereignArchonCore.h"

namespace UESP_Sovereign
{
    SovereignArchonCoreProcessor::SovereignArchonCoreProcessor(double sampleRate, PluginID targetModule)
        : sr(sampleRate), moduleID(targetModule), currentFrameClock(0),
          fetGateVoltage(0.0f), delayIndex(0), reverbIndex(0)
    {
        delayTapeLine.resize(static_cast<int>(sampleRate * 3.0), 0.0f);
        reverbFDNMesh.resize(32768, 0.0f);
        updateTMTChannelProfile(1);
    }

    void SovereignArchonCoreProcessor::updateTMTChannelProfile(int channelStripIndex)
    {
        unsigned int seed = static_cast<unsigned int>(channelStripIndex * 144);
        tmtNode.driftBiasL = 1.0f + (((seed % 200) - 100) * 0.00002f); 
        tmtNode.driftBiasR = 1.0f + (((seed % 200) - 100) * 0.00002f);
        tmtNode.internalToleranceScale = 1.0f + (((seed % 100) - 50) * 0.0001f);
        tmtNode.physicalCylinderID = (channelStripIndex % 12) + 1;
    }

    void SovereignArchonCoreProcessor::executeDSPBlock(float* left, float* right, int sampleCount)
    {
        const float A_Constant = 87.6f;
        const float aLawDenominator = 1.0f + std::log(A_Constant);
        const float spaceSSDScale = 2.0f / 7.0f;

        for (int i = 0; i < sampleCount; ++i)
        {
            float xL = left[i];
            float xR = right[i];

            float phaseAngle = (currentFrameClock % 72) * (2.0f * 3.14159265f / 72.0f);
            float coreModifier = std::sin(phaseAngle);

            // TMT Micro-Drift Phase Modulators
            float activeBiasL = tmtNode.driftBiasL + (std::sin(currentFrameClock * 0.00005f) * 0.001f);
            float activeBiasR = tmtNode.driftBiasR + (std::cos(currentFrameClock * 0.00004f) * 0.001f);

            switch (moduleID)
            {
                case COMPRESSOR: {
                    float energy = 0.5f * (std::abs(xL) + std::abs(xR));
                    if (energy > 0.05f) fetGateVoltage = (0.992f * fetGateVoltage) + (0.008f * energy);
                    else fetGateVoltage *= 0.9997f;

                    float reduction = 1.0f / (1.0f + 5.0f * fetGateVoltage * tmtNode.internalToleranceScale);
                    xL *= reduction; xR *= reduction;
                    xL = applyWhite72A(xL, A_Constant * activeBiasL, aLawDenominator);
                    xR = applyWhite72A(xR, A_Constant * activeBiasR, aLawDenominator);
                    break;
                }
                case SATURATION: {
                    float drive = 1.45f * tmtNode.internalToleranceScale;
                    float outL = applyHyperbolicCushion(xL * drive, activeBiasL);
                    float outR = applyHyperbolicCushion(xR * drive, activeBiasR);
                    float valveL = std::pow(std::abs(xL + 0.05f), 1.5f) * (xL >= -0.05f ? 1.0f : -1.0f);
                    float valveR = std::pow(std::abs(xR + 0.05f), 1.5f) * (xR >= -0.05f ? 1.0f : -1.0f);
                    xL = (1.0f - spaceSSDScale) * outL + spaceSSDScale * valveL;
                    xR = (1.0f - spaceSSDScale) * outR + spaceSSDScale * valveR;
                    break;
                }
                case REVERB: {
                    int tap = 16381;
                    int readIdx = (reverbIndex - tap + reverbFDNMesh.size()) % reverbFDNMesh.size();
                    float chorus = std::sin(currentFrameClock * 0.0015f * activeBiasL) * 8.0f;
                    int finalRead = (readIdx + static_cast<int>(chorus) + reverbFDNMesh.size()) % reverbFDNMesh.size();
                    float reflections = reverbFDNMesh[finalRead];
                    reverbFDNMesh[reverbIndex] = xL + (reflections * (0.65f - spaceSSDScale));
                    reverbIndex = (reverbIndex + 1) % reverbFDNMesh.size();
                    xL = (xL * 0.6f) + (reflections * 0.4f); xR = (xR * 0.6f) + (reflections * 0.4f);
                    break;
                }
                case DELAY: {
                    float wow = 1.0f + (0.002f * coreModifier * activeBiasL);
                    int offset = static_cast<int>(sr * 0.5f * wow * (1.0f + spaceSSDScale));
                    int readHead = (delayIndex - offset + delayTapeLine.size()) % delayTapeLine.size();
                    float feedback = delayTapeLine[readHead];
                    delayTapeLine[delayIndex] = xL + (feedback * 0.45f);
                    delayIndex = (delayIndex + 1) % delayTapeLine.size();
                    xL += feedback * 0.5f; xR += feedback * 0.5f;
                    break;
                }
                case EQUALIZER: {
                    float pultecAngle = 2.0f * 3.14159265f * 30.0f * activeBiasL * (static_cast<float>(i) / sr);
                    float filterW = std::sin(pultecAngle);
                    xL = xL + (xL * std::max(0.0f, filterW) * 0.12f) - (xL * std::max(0.0f, -filterW) * 0.08f);
                    for (int c = 1; c <= 12; ++c) {
                        if (c == tmtNode.physicalCylinderID) { xL = std::tanh(xL * 1.03f); xR *= 0.98f; }
                        else { xL *= (1.001f + 0.001f * coreModifier); }
                    }
                    break;
                }
                case MAXIMIZER: {
                    float crossoverAngle = 2.0f * 3.14159265f * 5000.0f * activeBiasR * (static_cast<float>(i) / sr);
                    float weight = std::abs(std::sin(crossoverAngle));
                    float hiL = xL * weight; float hiR = xR * weight;
                    xL += std::pow(hiL, 3.0f) * 0.25f * activeBiasL;
                    xR += std::pow(hiR, 3.0f) * 0.25f * activeBiasR;
                    break;
                }
            }

            currentFrameClock++;
            left[i] = std::clamp(xL, -1.0f, 1.0f);
            right[i] = std::clamp(xR, -1.0f, 1.0f);
        }
    }
}
