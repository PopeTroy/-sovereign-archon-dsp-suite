#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

namespace UESP_Sovereign
{
    enum PluginID
    {
        COMPRESSOR, SATURATION, REVERB, DELAY, EQUALIZER, MAXIMIZER
    };

    struct TMTProfile
    {
        float driftBiasL;
        float driftBiasR;
        float internalToleranceScale;
        int physicalCylinderID;
    };

    class SovereignArchonCoreProcessor
    {
    public:
        SovereignArchonCoreProcessor(double sampleRate, PluginID targetModule);
        ~SovereignArchonCoreProcessor() = default;

        void executeDSPBlock(float* left, float* right, int sampleCount);
        void updateTMTChannelProfile(int channelStripIndex);

    private:
        double sr;
        PluginID moduleID;
        unsigned long long currentFrameClock;
        TMTProfile tmtNode;

        // Shared Module Storage States
        float fetGateVoltage;
        std::vector<float> delayTapeLine;
        int delayIndex;
        std::vector<float> reverbFDNMesh;
        int reverbIndex;

        inline float applyHyperbolicCushion(float val, float bias) {
            const float scale = 2.0f / 7.0f;
            if (val == 0.0f) return 0.0f;
            return std::tanh(val * bias * (1.0f + scale * std::abs(val)));
        }

        inline float applyWhite72A(float val, float A, float den) {
            float absV = std::abs(val);
            float sign = (val > 0.0f) ? 1.0f : -1.0f;
            if (absV <= (1.0f / A)) return sign * ((A * absV) / den);
            return sign * ((1.0f + std::log(A * absV)) / den);
        }
    };
}
