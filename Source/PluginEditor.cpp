#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    buildInfo.setText (__DATE__ " " __TIME__ " ", juce::dontSendNotification);
    buildInfo.setFont (juce::FontOptions (12.0f));
    buildInfo.setJustificationType(juce::Justification::centredBottom);
    addAndMakeVisible(buildInfo);

    probabilitySliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processorRef.apvts, "Probability", probability);

    probability.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    probability.setRange(0.0f, 0.5f, 0.01f);
    probability.setDoubleClickReturnValue(true, 0.0f);
    probability.addListener(this);
    probability.setBounds(50, 30, 50, 200);

    addAndMakeVisible(probability);

    blockSizeSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processorRef.apvts, "Block Size", blockSize);

    blockSize.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    blockSize.setRange(1, 2400, 1);
    blockSize.setDoubleClickReturnValue(true, 1);
    blockSize.addListener(this);
    blockSize.setBounds(400, 30, 50, 200);

    addAndMakeVisible(blockSize);


    setSize (500, 300);
}

void AudioPluginAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &probability)
    {
        processorRef.probability = probability.getValue();
    }
    else if (slider == &blockSize)
    {
        processorRef.blockSize = blockSize.getValue();
    }
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World! g", getLocalBounds(), juce::Justification::centred, 1);
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor.
    buildInfo.setBounds (10, getHeight() - 30, getWidth() - 20, 20);
}
