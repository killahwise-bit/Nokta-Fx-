#pragma once

#include "PluginProcessor.h"

// Colores derivados de un matiz (hue) y una saturacion
struct NoktTheme
{
    float hue = 0.74f, sat = 0.80f;
    juce::Colour accent, accentLight, accentDim, text, dim, panel, bg;

    NoktTheme() { set (0.74f, 0.80f); }

    void set (float h, float s)
    {
        hue = h;
        sat = s;
        accent      = juce::Colour::fromHSV (h, s, 1.0f, 1.0f);
        accentLight = juce::Colour::fromHSV (h, s * 0.55f, 1.0f, 1.0f);
        accentDim   = juce::Colour::fromHSV (h, s, 0.45f, 1.0f);
        text        = juce::Colour::fromHSV (h, s * 0.22f, 0.93f, 1.0f);
        dim         = juce::Colour::fromHSV (h, s * 0.30f, 0.78f, 1.0f);
        panel       = juce::Colour::fromHSV (h, juce::jmin (1.0f, s * 0.6f), 0.14f, 1.0f);
        bg          = juce::Colour::fromHSV (h, juce::jmin (1.0f, s * 0.7f), 0.05f, 1.0f);
    }
};

class NoktLookAndFeel : public juce::LookAndFeel_V4
{
public:
    NoktLookAndFeel();

    void applyTheme (float hue, float sat);

    NoktTheme theme;

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float startAngle, float endAngle,
                           juce::Slider&) override;

    juce::Label* createSliderTextBox (juce::Slider&) override;

    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox&) override;

    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;
};

// Boton de menu (tres lineas)
class NoktMenuButton : public juce::Button
{
public:
    NoktMenuButton() : juce::Button ("menu") {}
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
};

class NoktFxEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit NoktFxEditor (NoktFxProcessor&);
    ~NoktFxEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void selectPreset (int index);
    void showMenu();
    void showHuePanel();
    void setTheme (float hue, float sat);

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<SliderAttachment> attachment;
    };

    NoktFxProcessor& proc;
    NoktLookAndFeel laf;
    std::array<Knob, 9> knobs;
    juce::ComboBox presetBox;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" };
    NoktMenuButton menuBtn;
    juce::Image logo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoktFxEditor)
};
