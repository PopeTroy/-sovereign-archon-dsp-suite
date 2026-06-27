#pragma once

#include <JuceHeader.h>

namespace UESP_Sovereign
{
    class SovereignArchonLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        SovereignArchonLookAndFeel();
        ~SovereignArchonLookAndFeel() override = default;

        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                              juce::Slider& slider) override;
    };
}
