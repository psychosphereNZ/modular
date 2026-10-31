#include "PluginProcessor.h"
#include "PluginEditor.h"

static constexpr float pi = juce::MathConstants<float>::pi;

IronModularAudioProcessor::IronModularAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "IRON_STATE", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout IronModularAudioProcessor::createParameterLayout()
{
    using P = juce::AudioParameterFloat;
    using C = juce::AudioParameterChoice;
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    auto addFloat = [&p](const char* id, const char* name, float lo, float hi, float def, float skew = 1.0f)
    {
        p.add (std::make_unique<P> (id, name, juce::NormalisableRange<float>(lo, hi, 0.001f, skew), def));
    };
    addFloat("osc1Level", "OSC 1 LEVEL", 0.0f, 1.0f, 0.75f);
    addFloat("osc2Level", "OSC 2 LEVEL", 0.0f, 1.0f, 0.45f);
    addFloat("subLevel", "SUB LEVEL", 0.0f, 1.0f, 0.25f);
    addFloat("noiseLevel", "NOISE", 0.0f, 0.5f, 0.0f);
    addFloat("tune2", "OSC 2 TUNE", -24.0f, 24.0f, 7.0f);
    addFloat("cutoff", "CUTOFF", 30.0f, 18000.0f, 900.0f, 0.35f);
    addFloat("resonance", "RESONANCE", 0.1f, 1.0f, 0.2f);
    addFloat("envAmount", "ENV AMOUNT", -1.0f, 1.0f, 0.35f);
    addFloat("attack", "ATTACK", 0.001f, 5.0f, 0.015f, 0.35f);
    addFloat("decay", "DECAY", 0.005f, 5.0f, 0.35f, 0.4f);
    addFloat("sustain", "SUSTAIN", 0.0f, 1.0f, 0.72f);
    addFloat("release", "RELEASE", 0.005f, 8.0f, 0.45f, 0.4f);
    addFloat("lfo1Rate", "LFO 1 RATE", 0.05f, 20.0f, 4.0f);
    addFloat("lfo1Amount", "LFO 1 TO PITCH", 0.0f, 1.0f, 0.0f);
    addFloat("lfo2Rate", "LFO 2 RATE", 0.05f, 20.0f, 0.35f);
    addFloat("lfo2Cutoff", "LFO 2 TO FILTER", 0.0f, 1.0f, 0.0f);
    addFloat("drive", "DRIVE", 0.0f, 1.0f, 0.08f);
    addFloat("delayMix", "DELAY MIX", 0.0f, 1.0f, 0.0f);
    addFloat("reverbMix", "REVERB MIX", 0.0f, 1.0f, 0.12f);
    addFloat("master", "MASTER", 0.0f, 1.0f, 0.8f);
    p.add (std::make_unique<C>("osc1Shape", "OSC 1 WAVE", juce::StringArray{"Saw","Square","Triangle","Sine"}, 0));
    p.add (std::make_unique<C>("osc2Shape", "OSC 2 WAVE", juce::StringArray{"Saw","Square","Triangle","Sine"}, 1));
    p.add (std::make_unique<C>("filterMode", "FILTER MODE", juce::StringArray{"Low Pass","Band Pass","High Pass"}, 0));
    return p;
}

void IronModularAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    filterL.reset(); filterR.reset();
    filterL.prepare({sampleRate, (juce::uint32)samplesPerBlock, 1});
    filterR.prepare({sampleRate, (juce::uint32)samplesPerBlock, 1});
    filterL.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    filterR.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    delayBuffer.setSize(2, (int)(sampleRate * 1.2) + samplesPerBlock);
    delayBuffer.clear(); delayWrite = 0;
    reverb.reset();
    for (auto& v : voices) v = Voice{};
}

void IronModularAudioProcessor::releaseResources() {}

bool IronModularAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono();
}

float IronModularAudioProcessor::oscillator(double& phase, float frequency, int shape, float pulseWidth)
{
    const float p = (float)phase;
    float out = 0.0f;
    if (shape == 0) out = 2.0f * p - 1.0f;
    else if (shape == 1) out = p < pulseWidth ? 1.0f : -1.0f;
    else if (shape == 2) out = 1.0f - 4.0f * std::abs(p - 0.5f);
    else out = std::sin(2.0f * pi * p);
    phase += (double)frequency / sr;
    phase -= std::floor(phase);
    return out;
}

void IronModularAudioProcessor::noteOn(int note, float velocity)
{
    auto* v = &voices[0];
    for (auto& candidate : voices) if (!candidate.active) { v = &candidate; break; }
    if (v->active) v = &*std::min_element(voices.begin(), voices.end(), [](const Voice& a, const Voice& b){ return a.env < b.env; });
    *v = Voice{};
    v->active = true; v->note = note; v->velocity = velocity; v->stage = Voice::Attack;
}
void IronModularAudioProcessor::noteOff(int note)
{
    for (auto& v : voices) if (v.active && v.note == note && v.stage != Voice::Off) v.stage = Voice::Release;
}
void IronModularAudioProcessor::allNotesOff()
{
    for (auto& v : voices) if (v.active) v.stage = Voice::Release;
}

float IronModularAudioProcessor::renderVoice(Voice& v, float lfo1, float lfo2, int)
{
    if (!v.active) return 0.0f;
    const float attack = *parameters.getRawParameterValue("attack");
    const float decay = *parameters.getRawParameterValue("decay");
    const float sustain = *parameters.getRawParameterValue("sustain");
    const float release = *parameters.getRawParameterValue("release");
    const float dt = 1.0f / (float)sr;
    switch (v.stage)
    {
        case Voice::Attack: v.env += dt / attack; if (v.env >= 1.0f) { v.env = 1.0f; v.stage = Voice::Decay; } break;
        case Voice::Decay: v.env -= dt * (1.0f - sustain) / decay; if (v.env <= sustain) { v.env = sustain; v.stage = Voice::Sustain; } break;
        case Voice::Sustain: v.env = sustain; break;
        case Voice::Release: v.env -= dt * juce::jmax(0.001f, v.env) / release; if (v.env <= 0.0001f) { v.env = 0.0f; v.active = false; v.stage = Voice::Off; return 0.0f; } break;
        default: return 0.0f;
    }

    const float pitchMod = lfo1 * *parameters.getRawParameterValue("lfo1Amount") * 12.0f;
    const float f1 = juce::MidiMessage::getMidiNoteInHertz(v.note + (int)std::round(pitchMod));
    const float f2 = juce::MidiMessage::getMidiNoteInHertz(v.note + (int)std::round(*parameters.getRawParameterValue("tune2")));
    const int shape1 = (int)*parameters.getRawParameterValue("osc1Shape");
    const int shape2 = (int)*parameters.getRawParameterValue("osc2Shape");
    float s = oscillator(v.phase1, f1, shape1, 0.5f) * *parameters.getRawParameterValue("osc1Level");
    s += oscillator(v.phase2, f2, shape2, 0.5f) * *parameters.getRawParameterValue("osc2Level");
    s += oscillator(v.subPhase, f1 * 0.5f, 1, 0.5f) * *parameters.getRawParameterValue("subLevel");
    s += (random.nextFloat() * 2.0f - 1.0f) * *parameters.getRawParameterValue("noiseLevel");
    const float cutoff = *parameters.getRawParameterValue("cutoff");
    const float envMod = *parameters.getRawParameterValue("envAmount") * v.env;
    const float lfoMod = lfo2 * *parameters.getRawParameterValue("lfo2Cutoff");
    const float modCutoff = juce::jlimit(30.0f, 18000.0f, cutoff * std::pow(2.0f, (envMod + lfoMod) * 4.0f));
    const float drive = 1.0f + *parameters.getRawParameterValue("drive") * 18.0f;
    s = std::tanh(s * drive) / std::tanh(drive);
    const float coeff = 1.0f - std::exp(-2.0f * pi * modCutoff / (float)sr);
    v.filterL += coeff * (s - v.filterL);
    const float resonance = *parameters.getRawParameterValue("resonance");
    s = v.filterL * (1.0f + resonance * 0.5f);
    return s * v.env * v.velocity * 0.22f;
}

void IronModularAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    auto it = midi.begin();
    int pos = 0;
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        while (it != midi.end() && (*it).samplePosition == sample)
        {
            const auto msg = (*it).getMessage();
            if (msg.isNoteOn()) noteOn(msg.getNoteNumber(), msg.getFloatVelocity());
            else if (msg.isNoteOff()) noteOff(msg.getNoteNumber());
            else if (msg.isAllNotesOff() || msg.isAllSoundOff()) allNotesOff();
            ++it;
        }
        const float lfo1 = std::sin((float)(2.0 * juce::MathConstants<double>::pi * lfoPhase1));
        const float lfo2 = std::sin((float)(2.0 * juce::MathConstants<double>::pi * lfoPhase2));
        lfoPhase1 += *parameters.getRawParameterValue("lfo1Rate") / sr; lfoPhase1 -= std::floor(lfoPhase1);
        lfoPhase2 += *parameters.getRawParameterValue("lfo2Rate") / sr; lfoPhase2 -= std::floor(lfoPhase2);
        float out = 0.0f;
        for (auto& voice : voices) out += renderVoice(voice, lfo1, lfo2, 0);
        out *= *parameters.getRawParameterValue("master");
        const float delayMix = *parameters.getRawParameterValue("delayMix");
        const int delaySamples = juce::jlimit(1, delayBuffer.getNumSamples()-1, (int)(sr * 0.28));
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            float delayed = delayBuffer.getSample(ch, (delayWrite - delaySamples + delayBuffer.getNumSamples()) % delayBuffer.getNumSamples());
            float dry = out;
            delayBuffer.setSample(ch, delayWrite, dry + delayed * 0.35f);
            buffer.setSample(ch, sample, dry * (1.0f - delayMix * 0.45f) + delayed * delayMix);
        }
        delayWrite = (delayWrite + 1) % delayBuffer.getNumSamples();
    }
    reverbParams.roomSize = 0.72f; reverbParams.damping = 0.45f; reverbParams.wetLevel = *parameters.getRawParameterValue("reverbMix") * 0.45f; reverbParams.dryLevel = 1.0f;
    reverb.setParameters(reverbParams);
    if (buffer.getNumChannels() == 2) reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());
    else reverb.processMono(buffer.getWritePointer(0), buffer.getNumSamples());
}

void IronModularAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}
void IronModularAudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessorEditor* IronModularAudioProcessor::createEditor() { return new IronModularAudioProcessorEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new IronModularAudioProcessor(); }
