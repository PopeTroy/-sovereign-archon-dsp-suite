#include "PluginProcessor.h"
#include "PluginEditor.h"

SovereignArchonSuiteAudioProcessorEditor::SovereignArchonSuiteAudioProcessorEditor (SovereignArchonSuiteAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (900, 580);

    auto setupNeonKnob = [this] (juce::Slider& knob, const juce::String& labelName) 
    {
        knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 75, 18);
        addAndMakeVisible (knob);
    };

    setupNeonKnob (inputGainKnob, "Input");
    setupNeonKnob (param1Knob, "Control I");
    setupNeonKnob (param2Knob, "Control II");
    setupNeonKnob (outputTrimKnob, "Output");
    setupNeonKnob (tmtChannelKnob, "TMT Desk ID");
    setupNeonKnob (stereoWidthKnob, "Width");

    // Configure Selector Rail
    auto setupBtn = [this](juce::TextButton& btn) {
        btn.setClickingTogglesState(true);
        btn.setRadioGroupId(1001);
        addAndMakeVisible(btn);
    };
    setupBtn(compressorModeBtn); setupBtn(saturationModeBtn); setupBtn(reverbModeBtn);
    setupBtn(delayModeBtn); setupBtn(eqModeBtn); setupBtn(maximizerModeBtn);
    compressorModeBtn.setToggleState(true, juce::dontSendNotification);

    startTimerHz (60);
}

SovereignArchonSuiteAudioProcessorEditor::~SovereignArchonSuiteAudioProcessorEditor() { stopTimer(); }

void SovereignArchonSuiteAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour (juce::Colour::fromString ("#F4F4F6")); // Pure Light Grey Substrate
    g.fillAll();

    // Secondary Glassmorphic Mask Window Frame
    g.setColour (juce::Colour::fromString ("#FFFFFF").withAlpha (0.55f));
    g.fillRoundedRectangle (bounds.reduced (15.0f), 10.0f);
    g.setColour (juce::Colour::fromString ("#2E3440").withAlpha (0.1f));
    g.drawRoundedRectangle (bounds.reduced (15.0f), 10.0f, 1.0f);

    // Dynamic Engine Explanation Sidebar Module Box
    auto infoArea = bounds.reduced(30.0f).removeFromBottom(100.0f);
    g.setColour (juce::Colour::fromString ("#2E3440").withAlpha (0.05f));
    g.fillRoundedRectangle(infoArea, 6.0f);

    g.setFont (juce::Font ("Courier New", 12.0f, juce::Font::plain));
    g.setColour (juce::Colour::fromString ("#4C566A"));
    
    juce::String explanationText = "ACTIVE MANIFOLD: ";
    switch (audioProcessor.getActiveMode())
    {
        case ARCHON_COMPRESSOR: explanationText += "FET Feedback Circuit + Logarithmic White 72A Gain Cushioning Math Matrices."; break;
        case ARCHON_SATURATION: explanationText += "Non-linear Hyperbolic Tangent Wave-shaper blended via 2/7 Light Matrix Constant."; break;
        case ARCHON_REVERB:     explanationText += "Prime-number Feedback Delay Network replicating mechanical EMT 140 plate acoustics."; break;
        case ARCHON_DELAY:      explanationText += "Motorized mechanical tape loop emulator driven by random phase flutter LFOs."; break;
        case ARCHON_EQUALIZER:  explanationText += "Parallel Pultec EQ low-end curve alignment with active 12-cylinder harmonic saturation."; break;
        case ARCHON_MAXIMIZER:  explanationText += "Zero-compromise hard-clipping brickwall limiter utilizing an odd-harmonic cubing algorithm."; break;
    }
    g.drawText(explanationText, infoArea.reduced(10.0f), juce::Justification::centredLeft, true);

    // PROMINENT POPE TROY SIGNATURE OVERLAY
    g.setFont (juce::Font ("Brush Script MT", 36.0f, juce::Font::italic));
    g.setColour (juce::Colour::fromString ("#FF3344")); // Premium Signature Ruby Red Tint
    g.drawText ("Pope Troy Signature Edition", 35, 30, 450, 40, juce::Justification::left);

    g.setFont (juce::Font ("Arial", 11.0f, juce::Font::bold));
    g.setColour (juce::Colour::fromString ("#2E3440").withAlpha(0.6f));
    g.drawText ("SOVEREIGN ARCHON 6-IN-1 ENGINE CORE // UESP PRCE", 35, 70, 500, 20, juce::Justification::left);

    // Oscillation Vector Plotter Grid
    auto waveArea = bounds.reduced(40.0f).removeFromTop(320.0f).removeFromBottom(120.0f);
    juce::Path waveTrace;
    waveTrace.startNewSubPath(waveArea.getX(), waveArea.getCentreY());
    for (float x = waveArea.getX(); x <= waveArea.getRight(); x += 2.0f)
    {
        float ratio = (x - waveArea.getX()) / waveArea.getWidth();
        float frequencyShift = 8.0f + (int)audioProcessor.getActiveMode() * 4.0f;
        float value = std::sin(ratio * frequencyShift + (float)juce::Time::getMillisecondCounterHiRes() * 0.005f);
        waveTrace.lineTo(x, waveArea.getCentreY() + (value * 35.0f * std::cos(ratio - 0.5f)));
    }
    g.setColour (juce::Colour::fromString ("#00FFCC").withAlpha(0.4f)); // Cyan Glow Accent
    g.strokePath(waveTrace, juce::PathStrokeType(3.0f));
}

void SovereignArchonSuiteAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (35);
    
    // Position Engine Configuration Rail Buttons
    auto topBar = area.removeFromTop(110).removeFromBottom(35);
    int btnWidth = topBar.getWidth() / 6;
    compressorModeBtn.setBounds(topBar.removeFromLeft(btnWidth).reduced(2));
    saturationModeBtn.setBounds(topBar.removeFromLeft(btnWidth).reduced(2));
    reverbModeBtn.setBounds(topBar.removeFromLeft(btnWidth).reduced(2));
    delayModeBtn.setBounds(topBar.removeFromLeft(btnWidth).reduced(2));
    eqModeBtn.setBounds(topBar.removeFromLeft(btnWidth).reduced(2));
    maximizerModeBtn.setBounds(topBar.reduced(2));

    // Position Expanded Dial Arrays
    auto knobZone = area.removeFromTop(200);
    int kw = knobZone.getWidth() / 6;
    inputGainKnob.setBounds(knobZone.removeFromLeft(kw).reduced(10));
    param1Knob.setBounds(knobZone.removeFromLeft(kw).reduced(10));
    param2Knob.setBounds(knobZone.removeFromLeft(kw).reduced(10));
    outputTrimKnob.setBounds(knobZone.removeFromLeft(kw).reduced(10));
    tmtChannelKnob.setBounds(knobZone.removeFromLeft(kw).reduced(10));
    stereoWidthKnob.setBounds(knobZone.reduced(10));
}

void SovereignArchonSuiteAudioProcessorEditor::timerCallback()
{
    // Check toggle statuses to dynamically route processing loops
    if (compressorModeBtn.getToggleState()) audioProcessor.setActiveMode(ARCHON_COMPRESSOR);
    else if (saturationModeBtn.getToggleState()) audioProcessor.setActiveMode(ARCHON_SATURATION);
    else if (reverbModeBtn.getToggleState()) audioProcessor.setActiveMode(ARCHON_REVERB);
    else if (delayModeBtn.getToggleState()) audioProcessor.setActiveMode(ARCHON_DELAY);
    else if (eqModeBtn.getToggleState()) audioProcessor.setActiveMode(ARCHON_EQUALIZER);
    else if (maximizerModeBtn.getToggleState()) audioProcessor.setActiveMode(ARCHON_MAXIMIZER);

    repaint();
}
