#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    const char* paramIds[9] = { "texture", "grit", "drift", "noise", "tone", "space", "width", "mix", "output" };
}

// Orden: texture, grit, drift, noise, tone, space, width, mix, output(dB)
const NoktFxProcessor::Preset NoktFxProcessor::presets[] =
{
    { "Polvo",      { 0.25f, 0.10f, 0.20f, 0.35f, -0.15f, 0.20f, 0.55f, 0.60f,  0.0f } },
    { "Cinta",      { 0.45f, 0.05f, 0.55f, 0.20f, -0.30f, 0.15f, 0.50f, 0.70f,  0.0f } },
    { "Vapor",      { 0.20f, 0.00f, 0.45f, 0.10f, -0.10f, 0.65f, 0.80f, 0.55f,  0.0f } },
    { "Gris",       { 0.60f, 0.55f, 0.05f, 0.10f,  0.10f, 0.05f, 0.50f, 0.65f, -2.0f } },
    { "Bruma",      { 0.15f, 0.00f, 0.30f, 0.25f, -0.45f, 0.55f, 0.70f, 0.50f,  0.0f } },
    { "Vacio",      { 0.10f, 0.05f, 0.15f, 0.00f, -0.25f, 0.90f, 1.00f, 0.45f, -1.0f } },
    { "Cristal",    { 0.10f, 0.20f, 0.10f, 0.05f,  0.45f, 0.35f, 0.85f, 0.45f,  0.0f } },
    { "Oxido",      { 0.70f, 0.70f, 0.35f, 0.30f, -0.20f, 0.10f, 0.45f, 0.75f, -3.0f } },
    { "Deriva",     { 0.30f, 0.10f, 0.90f, 0.15f, -0.05f, 0.30f, 0.75f, 0.65f,  0.0f } },
    { "Terciopelo", { 0.35f, 0.00f, 0.20f, 0.08f, -0.20f, 0.25f, 0.60f, 0.80f,  0.0f } },
    { "Ceniza",     { 0.40f, 0.30f, 0.10f, 0.35f, -0.35f, 0.10f, 0.45f, 0.70f, -1.0f } },
    { "Eclipse",    { 0.30f, 0.05f, 0.25f, 0.05f, -0.55f, 0.70f, 0.90f, 0.60f,  0.0f } },
    { "Neblina",    { 0.10f, 0.00f, 0.35f, 0.15f, -0.20f, 0.80f, 0.85f, 0.55f,  0.0f } },
    { "Cassette",   { 0.50f, 0.15f, 0.70f, 0.25f, -0.40f, 0.10f, 0.40f, 0.80f,  0.0f } },
    { "Radio",      { 0.55f, 0.40f, 0.15f, 0.45f,  0.35f, 0.05f, 0.20f, 0.85f, -2.0f } },
    { "Subsuelo",   { 0.65f, 0.25f, 0.20f, 0.10f, -0.70f, 0.35f, 0.60f, 0.75f, -2.0f } },
    { "Brasa",      { 0.80f, 0.20f, 0.25f, 0.15f, -0.10f, 0.15f, 0.55f, 0.70f, -4.0f } },
    { "Escarcha",   { 0.15f, 0.35f, 0.15f, 0.20f,  0.65f, 0.45f, 0.90f, 0.40f,  0.0f } },
    { "Sombra",     { 0.25f, 0.00f, 0.40f, 0.05f, -0.60f, 0.55f, 0.75f, 0.65f,  0.0f } },
    { "Pixel",      { 0.20f, 0.85f, 0.00f, 0.05f,  0.15f, 0.05f, 0.50f, 0.60f, -2.0f } },
    { "Humo",       { 0.35f, 0.10f, 0.50f, 0.30f, -0.35f, 0.60f, 0.80f, 0.60f,  0.0f } },
    { "Ecos",       { 0.15f, 0.00f, 0.60f, 0.00f, -0.15f, 0.95f, 1.00f, 0.50f, -1.0f } },
    { "Nostalgia",  { 0.40f, 0.20f, 0.65f, 0.40f, -0.50f, 0.30f, 0.55f, 0.75f, -1.0f } },
    { "Noche",      { 0.30f, 0.05f, 0.30f, 0.10f, -0.30f, 0.60f, 0.85f, 0.60f,  0.0f } },
    { "Cenit",      { 0.55f, 0.10f, 0.10f, 0.00f,  0.25f, 0.25f, 0.95f, 0.55f, -2.0f } },
};
const int NoktFxProcessor::numPresets = (int) (sizeof (presets) / sizeof (presets[0]));

juce::AudioProcessorValueTreeState::ParameterLayout NoktFxProcessor::createLayout()
{
    using P = juce::AudioParameterFloat;
    using R = juce::NormalisableRange<float>;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    auto add = [&] (const char* id, const char* name, float lo, float hi, float def)
    {
        p.push_back (std::make_unique<P> (juce::ParameterID { id, 1 }, name, R (lo, hi, 0.001f), def));
    };

    add ("texture", "Texture", 0.0f, 1.0f, 0.30f);
    add ("grit",    "Grit",    0.0f, 1.0f, 0.00f);
    add ("drift",   "Drift",   0.0f, 1.0f, 0.20f);
    add ("noise",   "Noise",   0.0f, 1.0f, 0.00f);
    add ("tone",    "Tone",   -1.0f, 1.0f, 0.00f);
    add ("space",   "Space",   0.0f, 1.0f, 0.20f);
    add ("width",   "Width",   0.0f, 1.0f, 0.50f);
    add ("mix",     "Mix",     0.0f, 1.0f, 0.70f);
    p.push_back (std::make_unique<P> (juce::ParameterID { "output", 1 }, "Output",
                                      R (-12.0f, 12.0f, 0.1f), 0.0f,
                                      juce::AudioParameterFloatAttributes().withLabel ("dB")));
    return { p.begin(), p.end() };
}

NoktFxProcessor::NoktFxProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
}

bool NoktFxProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void NoktFxProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;

    for (int c = 0; c < 2; ++c)
    {
        delayBuf[(size_t) c].assign ((size_t) maxDelaySamples, 0.0f);
        writePos[(size_t) c] = 0;
        lpState[(size_t) c] = hpState[(size_t) c] = crushHold[(size_t) c] = 0.0f;
    }
    noiseLp = 0.0f;
    crushCounter = 0.0f;
    driftCounter = 0;

    reverb.prepare ({ sampleRate, (juce::uint32) samplesPerBlock, 2 });
    reverb.reset();

    wetBuf.setSize (2, juce::jmax (1, samplesPerBlock));

    auto init = [&] (juce::SmoothedValue<float>& s, const char* id)
    {
        s.reset (sampleRate, 0.03);
        s.setCurrentAndTargetValue (apvts.getRawParameterValue (id)->load());
    };
    init (sTexture, "texture"); init (sGrit, "grit");   init (sDrift, "drift");
    init (sNoise,   "noise");   init (sTone, "tone");   init (sSpace, "space");
    init (sWidth,   "width");   init (sMix,  "mix");    init (sOut,   "output");
}

void NoktFxProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    if (buffer.getNumChannels() < 2 || n == 0)
        return;

    wetBuf.setSize (2, n, false, false, true);

    auto setT = [&] (juce::SmoothedValue<float>& s, const char* id)
    {
        s.setTargetValue (apvts.getRawParameterValue (id)->load());
    };
    for (int k = 0; k < 9; ++k)
    {
        juce::SmoothedValue<float>* sv[9] = { &sTexture, &sGrit, &sDrift, &sNoise, &sTone,
                                              &sSpace, &sWidth, &sMix, &sOut };
        setT (*sv[k], paramIds[k]);
    }

    const float twoPi = juce::MathConstants<float>::twoPi;
    const float fs = (float) sr;
    const float lfoInc = 0.35f / fs;

    float* dry[2] = { buffer.getWritePointer (0), buffer.getWritePointer (1) };
    float* wet[2] = { wetBuf.getWritePointer (0), wetBuf.getWritePointer (1) };

    // ---- Pasada 1: saturacion -> deriva -> grit -> ruido -> tono ----
    for (int i = 0; i < n; ++i)
    {
        const float tex   = sTexture.getNextValue();
        const float grit  = sGrit.getNextValue();
        const float drift = sDrift.getNextValue();
        const float noise = sNoise.getNextValue();
        const float tone  = sTone.getNextValue();

        // LFO suave + componente aleatoria lenta
        lfoPhase += lfoInc;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;
        if (--driftCounter <= 0)
        {
            driftCounter = (int) (sr * 0.25);
            driftRandomTarget = rng.nextFloat() * 2.0f - 1.0f;
        }
        driftRandom += (driftRandomTarget - driftRandom) * (5.0f / fs);
        const float m = 0.6f * std::sin (twoPi * lfoPhase) + 0.4f * driftRandom;
        const float depth = drift * 0.004f * fs;
        const float delaySamples = 1.0f + depth * (1.0f + m);

        // Saturacion
        const float drive = 1.0f + tex * 7.0f;
        const float driveNorm = 1.0f / std::sqrt (drive);

        // Grit (bitcrush + reduccion de muestreo)
        bool refresh = false;
        float levels = 1.0f;
        const bool gritOn = grit > 0.001f;
        if (gritOn)
        {
            crushCounter += 1.0f;
            const float factor = 1.0f + grit * grit * 14.0f;
            if (crushCounter >= factor) { crushCounter -= factor; refresh = true; }
            levels = std::pow (2.0f, (16.0f - grit * 11.0f) - 1.0f);
        }

        // Ruido: siseo suave (mono) + crujidos de polvo (por canal)
        const float white = rng.nextFloat() * 2.0f - 1.0f;
        noiseLp += 0.15f * (white - noiseLp);
        const float hiss = noiseLp * 1.5f * 0.08f * noise;

        // Coeficientes de tono
        float lpFc = tone < 0.0f ? 20000.0f * std::pow (0.03f, -tone) : 20000.0f;
        lpFc = juce::jmin (lpFc, 0.45f * fs);
        const float hpFc = tone > 0.0f ? 20.0f * std::pow (75.0f, tone) : 20.0f;
        const float aLp = 1.0f - std::exp (-twoPi * lpFc / fs);
        const float aHp = 1.0f - std::exp (-twoPi * hpFc / fs);

        for (int c = 0; c < 2; ++c)
        {
            const size_t ci = (size_t) c;
            float y = dry[c][i];

            if (tex > 0.001f)
                y = std::tanh (y * drive) * driveNorm;

            // Deriva (delay modulado)
            auto& d = delayBuf[ci];
            d[(size_t) writePos[ci]] = y;
            float rp = (float) writePos[ci] - delaySamples;
            if (rp < 0.0f) rp += (float) maxDelaySamples;
            const int i0 = (int) rp;
            const float fr = rp - (float) i0;
            int i1 = i0 + 1;
            if (i1 >= maxDelaySamples) i1 -= maxDelaySamples;
            y = d[(size_t) i0] + fr * (d[(size_t) i1] - d[(size_t) i0]);
            writePos[ci] = (writePos[ci] + 1) % maxDelaySamples;

            // Grit
            if (gritOn)
            {
                if (refresh) crushHold[ci] = std::round (y * levels) / levels;
                y = crushHold[ci];
            }

            // Ruido
            float crackle = 0.0f;
            if (noise > 0.001f && rng.nextFloat() < 0.00025f)
                crackle = (rng.nextFloat() * 2.0f - 1.0f) * 0.25f * noise;
            y += hiss + crackle;

            // Tono: pasa-bajos + pasa-altos de un polo
            lpState[ci] += aLp * (y - lpState[ci]);
            y = lpState[ci];
            hpState[ci] += aHp * (y - hpState[ci]);
            y -= hpState[ci];

            wet[c][i] = y;
        }
    }

    // ---- Space (reverb en la senal procesada) ----
    const float space = sSpace.skip (n);
    juce::dsp::Reverb::Parameters rvb;
    rvb.roomSize   = 0.55f + 0.40f * space;
    rvb.damping    = 0.55f;
    rvb.width      = 1.0f;
    rvb.wetLevel   = space * 0.55f;
    rvb.dryLevel   = 1.0f - space * 0.35f;
    rvb.freezeMode = 0.0f;
    reverb.setParameters (rvb);

    juce::dsp::AudioBlock<float> block (wetBuf);
    reverb.process (juce::dsp::ProcessContextReplacing<float> (block));

    // ---- Pasada 2: ancho, mezcla seco/humedo y salida ----
    for (int i = 0; i < n; ++i)
    {
        const float w   = sWidth.getNextValue();
        const float mix = sMix.getNextValue();
        const float g   = juce::Decibels::decibelsToGain (sOut.getNextValue());

        const float l = wet[0][i], r = wet[1][i];
        const float mid  = (l + r) * 0.5f;
        const float side = (l - r) * 0.5f * (w * 2.0f);
        const float wl = mid + side, wr = mid - side;

        dry[0][i] = (dry[0][i] * (1.0f - mix) + wl * mix) * g;
        dry[1][i] = (dry[1][i] * (1.0f - mix) + wr * mix) * g;
    }
}

// ---- Presets ----
int NoktFxProcessor::getNumPrograms() { return numPresets; }

const juce::String NoktFxProcessor::getProgramName (int index)
{
    if (index < 0 || index >= numPresets) return {};
    return presets[index].name;
}

void NoktFxProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= numPresets) return;
    currentPreset = index;
    for (int k = 0; k < 9; ++k)
        if (auto* prm = apvts.getParameter (paramIds[k]))
            prm->setValueNotifyingHost (prm->convertTo0to1 (presets[index].v[k]));
}

// ---- Estado ----
void NoktFxProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("preset", currentPreset, nullptr);
    state.setProperty ("hue", themeHue, nullptr);
    state.setProperty ("sat", themeSat, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void NoktFxProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto vt = juce::ValueTree::fromXml (*xml);
            currentPreset = juce::jlimit (0, numPresets - 1, (int) vt.getProperty ("preset", 0));
            themeHue = (float) (double) vt.getProperty ("hue", 0.74);
            themeSat = (float) (double) vt.getProperty ("sat", 0.80);
            apvts.replaceState (vt);
        }
}

juce::AudioProcessorEditor* NoktFxProcessor::createEditor()
{
    return new NoktFxEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NoktFxProcessor();
}
