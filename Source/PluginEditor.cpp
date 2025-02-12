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

    smoothSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processorRef.apvts, "Smooth", smooth);

    smooth.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    smooth.setRange(0.0f, 1.0f, 0.01f);
    smooth.setDoubleClickReturnValue(true, 0.0f);
    smooth.addListener(this);
    smooth.setBounds(150, 30, 50, 200);

    addAndMakeVisible(smooth);

    smoothModeSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processorRef.apvts, "SmoothMode", smoothMode);

    smoothMode.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    smoothMode.setRange(0.0f, 1.0f, 0.01f);
    smoothMode.setDoubleClickReturnValue(true, 0.0f);
    smoothMode.addListener(this);
    smoothMode.setBounds(320, 30, 35, 200);

    addAndMakeVisible(smoothMode);


    setSize (500, 300);
}

void AudioPluginAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &probability) {
        processorRef.probability = probability.getValue();
    } else if (slider == &blockSize) {
        processorRef.blockSize = blockSize.getValue();
    } else if (slider == &smooth) {
        processorRef.smooth = smooth.getValue();
    } else if (slider == &smoothMode) {
        processorRef.smoothMode = smoothMode.getValue();
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
    g.drawFittedText ("Hello World! s", getLocalBounds(), juce::Justification::centred, 1);
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor.
    buildInfo.setBounds (10, getHeight() - 30, getWidth() - 20, 20);
}
