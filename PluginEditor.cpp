#include "PluginEditor.h"

static const juce::Colour bg (0xff111315);
static const juce::Colour panel (0xff202427);
static const juce::Colour accent (0xffff6a2b);
static const juce::Colour textCol (0xffe7e2d9);
static const juce::Colour muted (0xff899096);

IronModularAudioProcessorEditor::IronModularAudioProcessorEditor(IronModularAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(1080, 690);
    addChoice("osc1Shape", "OSC 1 WAVE");
    addKnob("osc1Level", "LEVEL");
    addChoice("osc2Shape", "OSC 2 WAVE");
    addKnob("osc2Level", "LEVEL");
    addKnob("tune2", "TUNE");
    addKnob("subLevel", "SUB");
    addKnob("noiseLevel", "NOISE");
    addKnob("cutoff", "CUTOFF");
    addKnob("resonance", "RESONANCE");
    addChoice("filterMode", "FILTER");
    addKnob("envAmount", "ENV MOD");
    addKnob("attack", "ATTACK");
    addKnob("decay", "DECAY");
    addKnob("sustain", "SUSTAIN");
    addKnob("release", "RELEASE");
    addKnob("lfo1Rate", "LFO 1 RATE");
    addKnob("lfo1Amount", "PITCH AMT");
    addKnob("lfo2Rate", "LFO 2 RATE");
    addKnob("lfo2Cutoff", "FILTER AMT");
    addKnob("drive", "DRIVE");
    addKnob("delayMix", "DELAY");
    addKnob("reverbMix", "REVERB");
    addKnob("master", "MASTER");
}

void IronModularAudioProcessorEditor::addKnob(const juce::String& id, const juce::String& caption)
{
    auto* s = sliders.add(new juce::Slider());
    addAndMakeVisible(s);
    s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 68, 18);
    s->setColour(juce::Slider::rotarySliderFillColourId, accent);
    s->setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff42494d));
    s->setColour(juce::Slider::thumbColourId, textCol);
    s->setColour(juce::Slider::textBoxTextColourId, textCol);
    s->setColour(juce::Slider::textBoxBackgroundColourId, panel);
    s->setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    attachments.push_back(std::make_unique<Attachment>(processor.parameters, id, *s));
    auto* l = knobLabels.add(new juce::Label());
    addAndMakeVisible(l);
    l->setText(caption, juce::dontSendNotification);
    l->setJustificationType(juce::Justification::centred);
    l->setColour(juce::Label::textColourId, muted);
    l->setFont(juce::Font(10.5f, juce::Font::bold));
}

void IronModularAudioProcessorEditor::addChoice(const juce::String& id, const juce::String& caption)
{
    auto* c = combos.add(new juce::ComboBox());
    addAndMakeVisible(c);
    c->addItemList(id == "osc1Shape" || id == "osc2Shape"
        ? juce::StringArray{"SAW","SQUARE","TRIANGLE","SINE"}
        : juce::StringArray{"LOW PASS","BAND PASS","HIGH PASS"}, 1);
    c->setColour(juce::ComboBox::backgroundColourId, panel);
    c->setColour(juce::ComboBox::textColourId, textCol);
    c->setColour(juce::ComboBox::outlineColourId, accent.withAlpha(0.5f));
    comboAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.parameters, id, *c));
    auto* l = comboLabels.add(new juce::Label());
    addAndMakeVisible(l);
    l->setText(caption, juce::dontSendNotification);
    l->setJustificationType(juce::Justification::centred);
    l->setColour(juce::Label::textColourId, muted);
    l->setFont(juce::Font(10.5f, juce::Font::bold));
}

void IronModularAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    g.setColour(accent); g.fillRect(0, 0, getWidth(), 5);
    g.setColour(textCol); g.setFont(juce::Font(27.0f, juce::Font::bold));
    g.drawText("IRON MODULAR", 24, 18, 360, 36, juce::Justification::left);
    g.setColour(muted); g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText("SEMI-MODULAR SYNTHESIS SYSTEM  /  V1.0", 27, 52, 400, 18, juce::Justification::left);
    g.setColour(accent); g.drawText("AUDIO", getWidth()-115, 27, 80, 18, juce::Justification::right);
    g.setColour(juce::Colour(0xff343a3e)); g.drawLine(24, 79, getWidth()-24, 79, 1.0f);

    auto module = [&](int x, int y, int w, int h, const juce::String& name)
    {
        g.setColour(panel); g.fillRoundedRectangle((float)x, (float)y, (float)w, (float)h, 7.0f);
        g.setColour(accent); g.fillRect(x, y, w, 3);
        g.setColour(textCol); g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawText(name, x+10, y+8, w-20, 18, juce::Justification::left);
        g.setColour(juce::Colour(0xff394146)); g.drawLine((float)x+10, (float)y+31, (float)x+w-10, (float)y+31, 1.0f);
    };
    module(24, 91, 340, 190, "01  /  OSCILLATORS + MIXER");
    module(378, 91, 330, 190, "02  /  FILTER");
    module(722, 91, 334, 190, "03  /  MODULATION");
    module(24, 295, 510, 180, "04  /  ENVELOPE GENERATOR");
    module(548, 295, 508, 180, "05  /  FX + OUTPUT");
    g.setColour(muted); g.setFont(juce::Font(10.0f));
    g.drawText("PATCH BAY  •  ROUTING MATRIX AND CABLE PATCHING IN A FUTURE EXPANSION", 25, 505, 1030, 18, juce::Justification::centred);
    g.setColour(juce::Colour(0xff333a3e)); g.drawLine(24, 533, getWidth()-24, 533, 1.0f);
    g.setColour(muted); g.setFont(juce::Font(10.0f));
    g.drawText("MIDI INPUT  •  12-VOICE ENGINE  •  INTERNAL MODULATION  •  SAVE STATE WITH DAW PROJECT", 24, 645, 1030, 18, juce::Justification::centred);
}

void IronModularAudioProcessorEditor::resized()
{
    // Layout is intentionally explicit: module panels mirror the front-panel architecture.
    auto placeKnob = [&](int index, int x, int y, int w, int h)
    {
        if (index < sliders.size()) sliders[index]->setBounds(x, y, w, h);
        if (index < knobLabels.size()) knobLabels[index]->setBounds(x, y-15, w, 14);
    };
    auto placeCombo = [&](int index, int x, int y, int w, int h)
    {
        if (index < combos.size()) combos[index]->setBounds(x, y, w, h);
        if (index < comboLabels.size()) comboLabels[index]->setBounds(x, y-15, w, 14);
    };

    // Knobs are added in this order:
    // osc1 level, osc2 level, tune, sub, noise, cutoff, resonance, env amount,
    // ADSR x4, LFO x4, drive, delay, reverb, master.
    placeKnob(0, 40, 151, 76, 94); placeKnob(1, 188, 151, 76, 94);
    placeKnob(2, 270, 151, 76, 94); placeKnob(3, 40, 226, 76, 50);
    placeKnob(4, 188, 226, 76, 50);
    placeCombo(0, 40, 119, 112, 25); placeCombo(1, 188, 119, 112, 25);
    placeKnob(5, 397, 151, 78, 94); placeKnob(6, 490, 151, 78, 94); placeKnob(7, 583, 151, 78, 94);
    placeCombo(2, 397, 119, 112, 25);
    placeKnob(8, 44, 363, 82, 94); placeKnob(9, 158, 363, 82, 94);
    placeKnob(10, 272, 363, 82, 94); placeKnob(11, 386, 363, 82, 94);
    placeKnob(12, 744, 151, 68, 94); placeKnob(13, 832, 151, 68, 94);
    placeKnob(14, 920, 151, 68, 94); placeKnob(15, 1000, 151, 48, 94);
    placeKnob(16, 585, 363, 78, 94); placeKnob(17, 690, 363, 78, 94);
    placeKnob(18, 795, 363, 78, 94); placeKnob(19, 900, 363, 65, 94);
    placeKnob(20, 978, 363, 65, 94);
}
