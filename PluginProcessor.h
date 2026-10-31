#pragma once
#include <JuceHeader.h>
#include <array>

class IronModularAudioProcessor : public juce::AudioProcessor
{
public:
    IronModularAudioProcessor();
    ~IronModularAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "IRON MODULAR"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Init"; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;

private:
    struct Voice
    {
        bool active = false;
        int note = -1;
        float velocity = 0.0f;
        double phase1 = 0.0, phase2 = 0.0, subPhase = 0.0;
        float env = 0.0f;
        enum Stage { Off, Attack, Decay, Sustain, Release } stage = Off;
        float releaseStep = 0.0f;
        float filterL = 0.0f, filterR = 0.0f;
    };

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void noteOn (int note, float velocity);
    void noteOff (int note);
    float renderVoice (Voice&, float lfo1, float lfo2, int channel);
    float oscillator (double& phase, float frequency, int shape, float pulseWidth);
    void allNotesOff();

    std::array<Voice, 12> voices;
    double sr = 44100.0;
    double lfoPhase1 = 0.0, lfoPhase2 = 0.0;
    juce::Random random;
    juce::dsp::StateVariableTPTFilter<float> filterL, filterR;
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;
    juce::AudioBuffer<float> delayBuffer;
    int delayWrite = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IronModularAudioProcessor)
};
