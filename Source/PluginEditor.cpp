#include "PluginEditor.h"
#include "LogoData.h"

namespace
{
    const char* paramIds[9]   = { "texture", "grit", "drift", "noise", "tone", "space", "width", "mix", "output" };
    const char* paramNames[9] = { "TEXTURE", "GRIT", "DRIFT", "NOISE", "TONE", "SPACE", "WIDTH", "MIX", "OUTPUT" };

    struct ThemeDef { const char* name; float hue; float sat; };
    const ThemeDef themes[] =
    {
        { "Morado",   0.74f, 0.80f },
        { "Violeta",  0.80f, 0.75f },
        { "Azul",     0.60f, 0.80f },
        { "Cian",     0.50f, 0.80f },
        { "Verde",    0.38f, 0.75f },
        { "Lima",     0.24f, 0.80f },
        { "Ambar",    0.12f, 0.85f },
        { "Naranja",  0.06f, 0.85f },
        { "Rojo",     0.00f, 0.85f },
        { "Rosa",     0.93f, 0.65f },
        { "Magenta",  0.85f, 0.80f },
        { "Plata",    0.00f, 0.00f },
    };
    const int numThemes = (int) (sizeof (themes) / sizeof (themes[0]));

    // Dibuja el logo (PNG con transparencia) teñido con el color del tema
    void drawLogo (juce::Graphics& g, const juce::Image& img, juce::Rectangle<float> r,
                   juce::Colour accent, juce::Colour light, float alpha)
    {
        if (! img.isValid())
            return;

        g.setGradientFill (juce::ColourGradient (light.withAlpha (alpha), r.getX(), r.getY(),
                                                 accent.withAlpha (alpha), r.getRight(), r.getBottom(), false));
        g.drawImage (img, r, juce::RectanglePlacement::centred, true);
    }

    // Panel para elegir cualquier matiz
    class HuePanel : public juce::Component
    {
    public:
        HuePanel (float h, std::function<void (float)> cb) : hue (h), onHue (std::move (cb))
        {
            setSize (260, 44);
        }

        void paint (juce::Graphics& g) override
        {
            g.fillAll (juce::Colour (0xff14121b));
            auto bar = juce::Rectangle<float> (14.0f, ((float) getHeight() - 10.0f) * 0.5f,
                                               (float) getWidth() - 28.0f, 10.0f);
            juce::ColourGradient grad (juce::Colour::fromHSV (0.0f, 0.8f, 1.0f, 1.0f), bar.getX(), 0.0f,
                                       juce::Colour::fromHSV (1.0f, 0.8f, 1.0f, 1.0f), bar.getRight(), 0.0f, false);
            for (int i = 1; i < 6; ++i)
                grad.addColour ((double) i / 6.0, juce::Colour::fromHSV ((float) i / 6.0f, 0.8f, 1.0f, 1.0f));
            g.setGradientFill (grad);
            g.fillRoundedRectangle (bar, 5.0f);

            const float x = bar.getX() + hue * bar.getWidth();
            g.setColour (juce::Colours::white);
            g.fillEllipse (x - 8.0f, bar.getCentreY() - 8.0f, 16.0f, 16.0f);
        }

        void mouseDown (const juce::MouseEvent& e) override { update (e); }
        void mouseDrag (const juce::MouseEvent& e) override { update (e); }

    private:
        void update (const juce::MouseEvent& e)
        {
            hue = juce::jlimit (0.0f, 1.0f, (e.position.x - 14.0f) / ((float) getWidth() - 28.0f));
            repaint();
            if (onHue) onHue (hue);
        }

        float hue;
        std::function<void (float)> onHue;
    };
}

// ---------------- Look and feel ----------------
NoktLookAndFeel::NoktLookAndFeel()
{
    applyTheme (0.74f, 0.80f);
}

void NoktLookAndFeel::applyTheme (float hue, float sat)
{
    theme.set (hue, sat);

    setColour (juce::ComboBox::textColourId, theme.text);
    setColour (juce::PopupMenu::backgroundColourId, theme.panel);
    setColour (juce::PopupMenu::textColourId, theme.text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, theme.accentDim);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    setColour (juce::Slider::textBoxOutlineColourId, theme.accent);
    setColour (juce::Slider::textBoxBackgroundColourId, theme.panel.withAlpha (0.5f));
    setColour (juce::TextButton::textColourOffId, theme.accent);
    setColour (juce::TextButton::textColourOnId, theme.accent);
    setColour (juce::Label::textColourId, theme.dim);
}

void NoktLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                        float pos, float startAngle, float endAngle, juce::Slider& s)
{
    const float u = 0.5f; // 1 unidad de diseno = 0.5 px
    const auto c = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).getCentre();
    const float ringR = 71.0f * u, ringT = 8.0f * u, bodyR = 49.0f * u;

    const float angle = startAngle + pos * (endAngle - startAngle);
    const bool bipolar = s.getMinimum() < 0.0;
    const float a0 = bipolar ? startAngle + 0.5f * (endAngle - startAngle) : startAngle;

    auto pt = [&] (float a, float r)
    {
        return juce::Point<float> (c.x + r * std::sin (a), c.y - r * std::cos (a));
    };

    // Marcas
    g.setColour (theme.accent.withAlpha (0.35f));
    for (int i = 0; i <= 20; ++i)
    {
        auto p = pt (startAngle + (endAngle - startAngle) * (float) i / 20.0f, 82.0f * u);
        g.fillEllipse (p.x - 0.8f, p.y - 0.8f, 1.6f, 1.6f);
    }

    // Pista
    juce::Path track;
    track.addCentredArc (c.x, c.y, ringR, ringR, 0.0f, startAngle, endAngle, true);
    g.setColour (theme.panel.brighter (0.25f));
    g.strokePath (track, juce::PathStrokeType (ringT, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Valor
    const float lo = juce::jmin (a0, angle), hi = juce::jmax (a0, angle);
    if (hi - lo > 0.02f)
    {
        juce::Path val;
        val.addCentredArc (c.x, c.y, ringR, ringR, 0.0f, lo, hi, true);
        g.setColour (theme.accent.withAlpha (0.20f));
        g.strokePath (val, juce::PathStrokeType (ringT * 2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (theme.accent);
        g.strokePath (val, juce::PathStrokeType (ringT * 0.75f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    else
    {
        auto p = pt (a0, ringR);
        g.setColour (theme.accent);
        g.fillEllipse (p.x - 2.0f, p.y - 2.0f, 4.0f, 4.0f);
    }

    if (bipolar)
    {
        auto p = pt (a0, 82.0f * u);
        g.setColour (theme.accent);
        g.fillEllipse (p.x - 2.0f, p.y - 2.0f, 4.0f, 4.0f);
    }

    // Cuerpo
    juce::Path body;
    body.addEllipse (c.x - bodyR, c.y - bodyR, bodyR * 2.0f, bodyR * 2.0f);
    juce::DropShadow (juce::Colours::black.withAlpha (0.7f), 8, { 0, 3 }).drawForPath (g, body);

    g.setGradientFill (juce::ColourGradient (theme.panel.brighter (0.5f), c.x - bodyR * 0.6f, c.y - bodyR * 0.8f,
                                             theme.bg, c.x + bodyR * 0.6f, c.y + bodyR * 0.9f, false));
    g.fillPath (body);
    g.setColour (theme.text.withAlpha (0.30f));
    g.drawEllipse (c.x - bodyR, c.y - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.0f);

    // Indicador
    auto p0 = pt (angle, bodyR * 0.42f);
    auto p1 = pt (angle, bodyR * 0.88f);
    g.setColour (juce::Colours::white);
    g.drawLine (juce::Line<float> (p0, p1), 2.2f);
}

juce::Label* NoktLookAndFeel::createSliderTextBox (juce::Slider& s)
{
    auto* l = juce::LookAndFeel_V4::createSliderTextBox (s);
    l->setFont (juce::Font (juce::FontOptions (15.0f)));
    l->setJustificationType (juce::Justification::centred);
    l->setColour (juce::Label::outlineColourId, theme.accent.withAlpha (0.9f));
    l->setColour (juce::Label::textColourId, juce::Colours::white);
    l->setColour (juce::Label::backgroundColourId, theme.panel.withAlpha (0.6f));
    return l;
}

void NoktLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox&)
{
    auto r = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (0.5f);
    g.setColour (theme.panel.withAlpha (0.85f));
    g.fillRoundedRectangle (r, 9.0f);
    g.setColour (theme.accentDim);
    g.drawRoundedRectangle (r, 9.0f, 1.0f);

    juce::Path arrow;
    const float ax = (float) width - 22.0f, ay = (float) height * 0.5f;
    arrow.addTriangle (ax - 7.0f, ay - 3.5f, ax + 7.0f, ay - 3.5f, ax, ay + 4.5f);
    g.setColour (theme.accent);
    g.fillPath (arrow);
}

juce::Font NoktLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font (juce::FontOptions (16.0f));
}

juce::Font NoktLookAndFeel::getTextButtonFont (juce::TextButton&, int)
{
    return juce::Font (juce::FontOptions (20.0f, juce::Font::bold));
}

void NoktLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&,
                                            bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (down ? theme.accentDim : (highlighted ? theme.panel.brighter (0.2f) : theme.panel.withAlpha (0.85f)));
    g.fillRoundedRectangle (r, 9.0f);
    g.setColour (theme.accentDim);
    g.drawRoundedRectangle (r, 9.0f, 1.0f);
}

void NoktMenuButton::paintButton (juce::Graphics& g, bool highlighted, bool down)
{
    getLookAndFeel().drawButtonBackground (g, *this, juce::Colours::transparentBlack, highlighted, down);

    juce::Colour c = juce::Colours::white;
    if (auto* l = dynamic_cast<NoktLookAndFeel*> (&getLookAndFeel()))
        c = l->theme.accent;

    auto r = getLocalBounds().toFloat();
    const float w = r.getWidth() * 0.36f, cx = r.getCentreX(), cy = r.getCentreY(), gap = r.getHeight() * 0.14f;
    g.setColour (c);
    for (int i = -1; i <= 1; ++i)
        g.fillRoundedRectangle (cx - w * 0.5f, cy + (float) i * gap - 1.0f, w, 2.0f, 1.0f);
}

// ---------------- Editor ----------------
NoktFxEditor::NoktFxEditor (NoktFxProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    laf.applyTheme (proc.themeHue, proc.themeSat);
    setLookAndFeel (&laf);
    logo = juce::ImageCache::getFromMemory (NoktLogo::data, NoktLogo::size);
    setSize (910, 432);

    for (size_t k = 0; k < knobs.size(); ++k)
    {
        auto& kb = knobs[k];
        kb.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        kb.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 22);
        kb.slider.setRotaryParameters (juce::degreesToRadians (225.0f), juce::degreesToRadians (495.0f), true);

        const int kind = (int) k;
        kb.slider.textFromValueFunction = [kind] (double v)
        {
            if (kind == 8) return juce::String (v, 1);
            return juce::String (v, 3);
        };

        kb.attachment = std::make_unique<SliderAttachment> (proc.apvts, paramIds[k], kb.slider);
        if (auto* prm = proc.apvts.getParameter (paramIds[k]))
            kb.slider.setDoubleClickReturnValue (true, prm->convertFrom0to1 (prm->getDefaultValue()));

        kb.label.setText (paramNames[k], juce::dontSendNotification);
        kb.label.setJustificationType (juce::Justification::centred);
        kb.label.setFont (juce::Font (juce::FontOptions (13.0f)));

        addAndMakeVisible (kb.slider);
        addAndMakeVisible (kb.label);
    }

    for (int i = 0; i < proc.getNumPrograms(); ++i)
        presetBox.addItem (proc.getProgramName (i), i + 1);
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this] { selectPreset (presetBox.getSelectedId() - 1); };
    addAndMakeVisible (presetBox);

    prevBtn.onClick = [this]
    {
        const int n = proc.getNumPrograms();
        selectPreset ((proc.getCurrentProgram() + n - 1) % n);
    };
    nextBtn.onClick = [this]
    {
        selectPreset ((proc.getCurrentProgram() + 1) % proc.getNumPrograms());
    };
    addAndMakeVisible (prevBtn);
    addAndMakeVisible (nextBtn);

    menuBtn.onClick = [this] { showMenu(); };
    addAndMakeVisible (menuBtn);

    startTimerHz (10);
}

NoktFxEditor::~NoktFxEditor()
{
    setLookAndFeel (nullptr);
}

void NoktFxEditor::selectPreset (int index)
{
    proc.setCurrentProgram (index);
    presetBox.setSelectedId (index + 1, juce::dontSendNotification);
}

void NoktFxEditor::timerCallback()
{
    const int want = proc.getCurrentProgram() + 1;
    if (presetBox.getSelectedId() != want)
        presetBox.setSelectedId (want, juce::dontSendNotification);
}

void NoktFxEditor::setTheme (float hue, float sat)
{
    proc.themeHue = hue;
    proc.themeSat = sat;
    laf.applyTheme (hue, sat);
    sendLookAndFeelChange();
    repaint();
}

void NoktFxEditor::showMenu()
{
    juce::PopupMenu colours;
    for (int i = 0; i < numThemes; ++i)
    {
        const bool current = std::abs (themes[i].hue - laf.theme.hue) < 0.001f
                          && std::abs (themes[i].sat - laf.theme.sat) < 0.001f;
        colours.addItem (i + 1, themes[i].name, true, current);
    }

    juce::PopupMenu m;
    m.addSubMenu ("Color", colours);
    m.addItem (100, "Matiz personalizado...");

    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&menuBtn),
                     [safe = juce::Component::SafePointer<NoktFxEditor> (this)] (int r)
                     {
                         auto* ed = safe.getComponent();
                         if (ed == nullptr || r == 0) return;
                         if (r == 100) ed->showHuePanel();
                         else          ed->setTheme (themes[r - 1].hue, themes[r - 1].sat);
                     });
}

void NoktFxEditor::showHuePanel()
{
    auto panel = std::make_unique<HuePanel> (laf.theme.hue,
        [safe = juce::Component::SafePointer<NoktFxEditor> (this)] (float h)
        {
            if (auto* ed = safe.getComponent())
                ed->setTheme (h, 0.80f);
        });

    juce::CallOutBox::launchAsynchronously (std::move (panel), menuBtn.getScreenBounds(), nullptr);
}

void NoktFxEditor::paint (juce::Graphics& g)
{
    const auto& t = laf.theme;
    g.fillAll (t.bg);

    // Todo se dibuja en un espacio de diseno de 1821 x 864 (escala 0.5)
    g.addTransform (juce::AffineTransform::scale (0.5f));
    const float W = 1821.0f, H = 864.0f;

    // Resplandor central
    g.setGradientFill (juce::ColourGradient (t.accent.withAlpha (0.10f), W * 0.5f, 330.0f,
                                             t.accent.withAlpha (0.0f), W * 0.5f, 900.0f, true));
    g.fillRect (0.0f, 0.0f, W, H);

    // Ondas de fondo
    {
        juce::Path sw;
        sw.startNewSubPath (0.0f, 0.0f);
        sw.lineTo (330.0f, 0.0f);
        sw.cubicTo (290.0f, 100.0f, 160.0f, 205.0f, 0.0f, 262.0f);
        sw.closeSubPath();
        g.setGradientFill (juce::ColourGradient (t.accent.withAlpha (0.16f), 330.0f, 0.0f,
                                                 t.accent.withAlpha (0.0f), 0.0f, 262.0f, false));
        g.fillPath (sw);

        juce::Path rw;
        rw.startNewSubPath (W, 175.0f);
        rw.cubicTo (1640.0f, 160.0f, 1470.0f, 230.0f, 1370.0f, 400.0f);
        rw.cubicTo (1310.0f, 505.0f, 1250.0f, 560.0f, 1190.0f, 600.0f);
        juce::Path rwFill (rw);
        rwFill.lineTo (W, 600.0f);
        rwFill.closeSubPath();
        g.setGradientFill (juce::ColourGradient (t.accent.withAlpha (0.0f), 1190.0f, 600.0f,
                                                 t.accent.withAlpha (0.16f), W, 175.0f, false));
        g.fillPath (rwFill);
        g.setColour (t.accent.withAlpha (0.45f));
        g.strokePath (rw, juce::PathStrokeType (3.0f));

        juce::Path br;
        br.startNewSubPath (W, 590.0f);
        br.cubicTo (1740.0f, 720.0f, 1560.0f, 800.0f, 1250.0f, 830.0f);
        juce::Path brFill (br);
        brFill.lineTo (1250.0f, H);
        brFill.lineTo (W, H);
        brFill.closeSubPath();
        g.setGradientFill (juce::ColourGradient (t.accent.withAlpha (0.16f), W, 590.0f,
                                                 t.accent.withAlpha (0.0f), 1250.0f, H, false));
        g.fillPath (brFill);
        g.setColour (t.accent.withAlpha (0.40f));
        g.strokePath (br, juce::PathStrokeType (3.0f));

        juce::Path bl;
        bl.startNewSubPath (0.0f, 690.0f);
        bl.cubicTo (140.0f, 740.0f, 300.0f, 800.0f, 640.0f, 830.0f);
        juce::Path blFill (bl);
        blFill.lineTo (640.0f, H);
        blFill.lineTo (0.0f, H);
        blFill.closeSubPath();
        g.setGradientFill (juce::ColourGradient (t.accent.withAlpha (0.14f), 0.0f, 690.0f,
                                                 t.accent.withAlpha (0.0f), 640.0f, H, false));
        g.fillPath (blFill);
        g.setColour (t.accent.withAlpha (0.35f));
        g.strokePath (bl, juce::PathStrokeType (3.0f));
    }

    // Logo grande, muy tenue, al centro
    drawLogo (g, logo, { 765.0f, 40.0f, 300.0f, 310.0f }, t.accent, t.accentLight, 0.16f);

    // Cabecera: logo, divisor y titulo
    drawLogo (g, logo, { 66.0f, 34.0f, 96.0f, 96.0f }, t.accent, t.accentLight, 0.95f);
    g.setColour (t.dim.withAlpha (0.7f));
    g.fillRect (183.0f, 55.0f, 2.0f, 70.0f);

    juce::AttributedString title;
    title.append ("Nokta-", juce::Font (juce::FontOptions (56.0f)).withExtraKerningFactor (0.04f), t.text);
    title.append ("Fx", juce::Font (juce::FontOptions (56.0f, juce::Font::bold)).withExtraKerningFactor (0.04f), t.accent);
    title.draw (g, juce::Rectangle<float> (222.0f, 46.0f, 600.0f, 90.0f));

    // Linea de cabecera con muesca
    {
        juce::Path hp;
        hp.startNewSubPath (0.0f, 158.0f);
        hp.lineTo (485.0f, 158.0f);
        hp.lineTo (508.0f, 137.0f);
        hp.lineTo (650.0f, 137.0f);
        hp.lineTo (670.0f, 115.0f);
        hp.lineTo (1080.0f, 115.0f);
        hp.lineTo (1125.0f, 158.0f);
        hp.lineTo (W, 158.0f);

        juce::ColourGradient lg (t.accent.withAlpha (0.0f), 0.0f, 0.0f, t.accent.withAlpha (0.0f), W, 0.0f, false);
        lg.addColour (0.18, t.accent.withAlpha (0.9f));
        lg.addColour (0.82, t.accent.withAlpha (0.9f));
        g.setGradientFill (lg);
        g.strokePath (hp.createPathWithRoundedCorners (10.0f), juce::PathStrokeType (2.5f));
    }

    // Pie con muesca y nombre
    {
        juce::Path fp;
        fp.startNewSubPath (0.0f, 772.0f);
        fp.lineTo (750.0f, 772.0f);
        fp.lineTo (772.0f, 752.0f);
        fp.lineTo (1050.0f, 752.0f);
        fp.lineTo (1072.0f, 772.0f);
        fp.lineTo (W, 772.0f);

        juce::ColourGradient lg (t.accent.withAlpha (0.0f), 0.0f, 0.0f, t.accent.withAlpha (0.0f), W, 0.0f, false);
        lg.addColour (0.15, t.accent.withAlpha (0.85f));
        lg.addColour (0.85, t.accent.withAlpha (0.85f));
        g.setGradientFill (lg);
        g.strokePath (fp.createPathWithRoundedCorners (8.0f), juce::PathStrokeType (2.5f));

        g.setColour (t.dim);
        g.setFont (juce::Font (juce::FontOptions (24.0f)).withExtraKerningFactor (0.6f));
        g.drawText ("NOKTA-FX", juce::Rectangle<float> (700.0f, 776.0f, 421.0f, 40.0f), juce::Justification::centred);
    }
}

void NoktFxEditor::resized()
{
    auto D = [] (float v) { return juce::roundToInt (v * 0.5f); };

    prevBtn.setBounds   (D (1030.0f), D (52.0f), D (73.0f),  D (73.0f));
    presetBox.setBounds (D (1118.0f), D (52.0f), D (442.0f), D (73.0f));
    nextBtn.setBounds   (D (1575.0f), D (52.0f), D (73.0f),  D (73.0f));
    menuBtn.setBounds   (D (1694.0f), D (52.0f), D (74.0f),  D (73.0f));

    const float cx[9] = { 168.0f, 353.0f, 537.0f, 722.0f, 906.0f, 1090.0f, 1274.0f, 1458.0f, 1642.0f };
    for (size_t k = 0; k < knobs.size(); ++k)
    {
        knobs[k].slider.setBounds (D (cx[k] - 92.0f), D (295.0f), D (184.0f), D (314.0f));
        knobs[k].label.setBounds  (D (cx[k] - 92.0f), D (622.0f), D (184.0f), D (38.0f));
    }
}
