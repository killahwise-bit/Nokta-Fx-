#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class NoktFxProcessor : public juce::AudioProcessor
{
public:
    NoktFxProcessor();
    ~NoktFxProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Nokta-Fx"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    // Presets
    int getNumPrograms() override;
    int getCurrentProgram() override { return currentPreset; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // Tema de color de la interfaz (se guarda en el estado)
    float themeHue = 0.74f;
    float themeSat = 0.80f;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    // Orden: texture, grit, drift, noise, tone, space, width, mix, output
    struct Preset { const char* name; float v[9]; };
    static const Preset presets[];
    static const int numPresets;
    int currentPreset = 0;

    // DSP
    static constexpr int maxDelaySamples = 4096;
    std::array<std::vector<float>, 2> delayBuf;
    std::array<int, 2> writePos { 0, 0 };
    float lfoPhase = 0.0f;
    float driftRandom = 0.0f, driftRandomTarget = 0.0f;
    int driftCounter = 0;

    std::array<float, 2> lpState { 0.0f, 0.0f };
    std::array<float, 2> hpState { 0.0f, 0.0f };
    std::array<float, 2> crushHold { 0.0f, 0.0f };
    float crushCounter = 0.0f;
    float noiseLp = 0.0f;
    juce::Random rng;

    juce::dsp::Reverb reverb;
    juce::AudioBuffer<float> wetBuf;
    juce::SmoothedValue<float> sTexture, sGrit, sDrift, sNoise, sTone, sSpace, sWidth, sMix, sOut;
    double sr = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoktFxProcessor)
};
