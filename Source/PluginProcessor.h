#pragma once

#include "juce_audio_processors/juce_audio_processors.h"

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    double probability = 0.0f;
    int16_t blockSize = 1;
    int32_t counter = 0;
    std::vector<std::vector<float>> futureSamples;
    int32_t writePtr = 0;
    double smooth = 0.0;
    int32_t latencySamples = 512;
    bool invert = false;
    bool previousInvert = false;

    int16_t smoothCounter = 0;
    int16_t smoothTargetSamples = 0;
    float smoothEndValue = 0.0f;
    float smoothStartValue = 0.0f;

    double smoothMode = 0.0;

    enum class State {
        NORMAL, SMOOTH
    };

    State state;
private:
    //==============================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::String ("probability"), juce::String ("Probability"), 0.0f, 0.5f, 0.0f));
        layout.add (std::make_unique<juce::AudioParameterInt>(juce::String("block size"), juce::String("Block Size"), 1, 512, 1));
        layout.add (std::make_unique<juce::AudioParameterFloat>(juce::String("smooth"), juce::String("Smooth"), 0.0f, 1.0f, 0.0f));
        layout.add (std::make_unique<juce::AudioParameterFloat>(juce::String("smooth mode"), juce::String("SmoothMode"), 0.0f, 1.0f, 0.0f));
        layout.add (std::make_unique<juce::AudioParameterFloat>(juce::String("mix"), juce::String("Mix"), 0.0f, 1.0f, 1.0f));
        layout.add (std::make_unique<juce::AudioParameterBool>(juce::String("bypass"), juce::String("Bypass"), false));

        return layout;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
