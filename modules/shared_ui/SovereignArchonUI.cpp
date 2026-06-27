#include "SovereignArchonUI.h"

namespace UESP_Sovereign
{
    SovereignArchonLookAndFeel::SovereignArchonLookAndFeel()
    {
        // Define clean light grey palette foundations
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour::fromString("#DCDCDC"));
        setColour(juce::Slider::thumbColourId, juce::Colour::fromString("#00FFCC"));
    }

    void SovereignArchonLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                                      float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                                      juce::Slider& slider)
    {
        auto radius = std::min(width, height) / 2.0f - 10.0f;
        auto centreX = x + width / 2.0f;
        auto centreY = y + height / 2.0f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // 1. Draw Glassmorphism Frosted Dial Base Base Panel
        g.setColour(juce::Colour::fromString("#FFFFFF").withAlpha(0.35f));
        g.fillEllipse(rx, ry, rw, rw);
        g.setColour(juce::Colour::fromString("#FFFFFF").withAlpha(0.60f));
        g.drawEllipse(rx, ry, rw, rw, 1.5f);

        // 2. Render Active Neon Emission Glow Indicator Arcs
        juce::Path glowPath;
        glowPath.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
        
        g.setColour(juce::Colour::fromString("#00FFCC").withAlpha(0.15f));
        g.strokePath(glowPath, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(juce::Colour::fromString("#00FFCC").withAlpha(0.40f));
        g.strokePath(glowPath, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        // Pointer Line Dot Core
        g.setColour(juce::Colour::fromString("#FFFFFF"));
        g.fillEllipse(centreX + (radius - 4.0f) * std::cos(angle - juce::MathConstants<float>::halfPi) - 2.0f,
                      centreY + (radius - 4.0f) * std::sin(angle - juce::MathConstants<float>::halfPi) - 2.0f, 4.0f, 4.0f);
    }
}
