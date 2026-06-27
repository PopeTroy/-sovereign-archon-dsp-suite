#include "PluginProcessor.h"
#include "PluginEditor.h"

SovereignArchonSuiteAudioProcessorEditor::SovereignArchonSuiteAudioProcessorEditor (SovereignArchonSuiteAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (850, 520);

    auto setupNeonKnob = [this] (juce::Slider& knob, const juce::String& suffix) 
    {
        knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        knob.setTextValueSuffix (suffix);
        addAndMakeVisible (knob);
    };

    setupNeonKnob (thresholdKnob, " dB");
    setupNeonKnob (ratioKnob, " :1");
    setupNeonKnob (mixKnob, " %");

    startTimerHz (60);
}

SovereignArchonSuiteAudioProcessorEditor::~SovereignArchonSuiteAudioProcessorEditor()
{
    stopTimer();
}

void SovereignArchonSuiteAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Glassmorphism Light Grey Backing Substrate
    g.setColour (juce::Colour::fromString ("#EAEAEA"));
    g.fillAll();

    g.setColour (juce::Colour::fromString ("#FFFFFF").withAlpha (0.42f));
    g.fillRoundedRectangle (bounds.reduced (20.0f), 12.0f);

    g.setColour (juce::Colour::fromString ("#FFFFFF").withAlpha (0.65f));
    g.drawRoundedRectangle (bounds.reduced (20.0f), 12.0f, 1.5f);

    auto width = getWidth();
    auto height = getHeight();
    auto midY = height / 2.0f;

    // Glowing Neon Cyan Visualizer Line
    juce::Path neonVisualizerPath;
    neonVisualizerPath.startNewSubPath (40.0f, midY + 80.0f);
    int activePoints = 64;
    float graphWidth = width - 80.0f;

    for (int i = 0; i < activePoints; ++i)
    {
        float progress = (float)i / (float)activePoints;
        float xPos = 40.0f + (progress * graphWidth);
        float waveOscillation = std::sin (progress * 12.0f + (float)juce::Time::getMillisecondCounterHiRes() * 0.006f);
        float yPos = (midY + 80.0f) + (waveOscillation * 45.0f * std::cos (progress * 3.14159f));
        neonVisualizerPath.lineTo (xPos, yPos);
    }

    g.setColour (juce::Colour::fromString ("#00FFCC").withAlpha (0.12f));
    g.strokePath (neonVisualizerPath, juce::PathStrokeType (8.0f));
    g.setColour (juce::Colour::fromString ("#00FFCC").withAlpha (0.35f));
    g.strokePath (neonVisualizerPath, juce::PathStrokeType (4.0f));
    g.setColour (juce::Colour::fromString ("#FFFFFF"));
    g.strokePath (neonVisualizerPath, juce::PathStrokeType (1.5f));

    // Structural Metadata Fine Print Title Overlay
    g.setFont (juce::Font ("Courier New", 13.0f, juce::Font::bold));
    g.setColour (juce::Colour::fromString ("#2E3440").withAlpha (0.75f));
    g.drawText ("SYS_CATALYST // ENGINE: QUANTUM_ARCHON_MANIFOLD", 35, 30, 500, 20, juce::Justification::left);

    // Branding Footprint
    int footerY = height - 45;
    int footerWidth = width - 70;

    // Pope Troy Cursive Red Script Branding
    g.setFont (juce::Font ("Brush Script MT", 24.0f, juce::Font::italic));
    g.setColour (juce::Colour::fromString ("#FF3344"));
    g.drawText ("Pope Troy Signature", footerWidth - 190, footerY - 5, 200, 30, juce::Justification::left);

    // Fine Print UESP PRCE Anchor
    g.setFont (juce::Font ("Arial", 10.0f, juce::Font::plain));
    g.setColour (juce::Colour::fromString ("#4C566A").withAlpha (0.65f));
    g.drawText ("|  UESP PRCE", footerWidth + 25, footerY + 4, 110, 20, juce::Justification::left);
}

void SovereignArchonSuiteAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (40);
    auto knobRowArea = area.removeFromTop (160);
    int knobWidth = knobRowArea.getWidth() / 3;

    thresholdKnob.setBounds (knobRowArea.removeFromLeft (knobWidth).reduced (15));
    ratioKnob.setBounds (knobRowArea.removeFromLeft (knobWidth).reduced (15));
    mixKnob.setBounds (knobRowArea.reduced (15));
}

void SovereignArchonSuiteAudioProcessorEditor::timerCallback()
{
    repaint();
}
