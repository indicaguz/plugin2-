#include "PluginProcessor.h"

GuzProcessor::GuzProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout GuzProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "gain", 1 }, "Volumen",
        juce::NormalisableRange<float> (0.0f, 2.0f, 0.01f), 1.0f));
    return layout;
}

bool GuzProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet()
        && (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
            || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono());
}

void GuzProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    const float gain = apvts.getRawParameterValue ("gain")->load();

    // rampa suave para evitar "clicks" al mover la perilla
    buffer.applyGainRamp (0, buffer.getNumSamples(), lastGain, gain);
    lastGain = gain;
}

juce::AudioProcessorEditor* GuzProcessor::createEditor()
{
    return new GuzEditor (*this);
}

void GuzProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void GuzProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

// ---------------- Interfaz ----------------

GuzEditor::GuzEditor (GuzProcessor& p)
    : AudioProcessorEditor (&p),
      attachment (p.apvts, "gain", slider)
{
    titulo.setText ("indica guz", juce::dontSendNotification);
    titulo.setFont (juce::FontOptions (30.0f, juce::Font::bold));
    titulo.setJustificationType (juce::Justification::centred);
    titulo.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (titulo);

    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 22);
    addAndMakeVisible (slider);

    etiqueta.setText ("Volumen", juce::dontSendNotification);
    etiqueta.setJustificationType (juce::Justification::centred);
    etiqueta.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible (etiqueta);

    setSize (300, 320);
}

void GuzEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1e1e2a));
}

void GuzEditor::resized()
{
    auto area = getLocalBounds().reduced (20);
    titulo.setBounds (area.removeFromTop (50));
    etiqueta.setBounds (area.removeFromBottom (24));
    slider.setBounds (area);
}

// Esta función le dice al DAW cómo crear el plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GuzProcessor();
}
