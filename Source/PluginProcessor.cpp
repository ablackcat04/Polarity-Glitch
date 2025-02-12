#include <random>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#define PI 3.141592653589793238L

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    int channels = getTotalNumInputChannels();
    std::vector<std::vector<float>> fs(channels, std::vector<float>(latencySamples + 1));
    futureSamples = fs;
    setLatencySamples(latencySamples);
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    auto playHead = getPlayHead();
    juce::Optional<int64_t> pos = playHead->getPosition()->getTimeInSamples();

    double probability = *apvts.getRawParameterValue("probability");
    double smooth = *apvts.getRawParameterValue("smooth");
    double smoothMode = *apvts.getRawParameterValue("smooth mode");
    int16_t blockSize = *apvts.getRawParameterValue("block size");
    double mix = *apvts.getRawParameterValue("mix");
    bool bypass = *apvts.getRawParameterValue("bypass");

    if (pos.hasValue()) {
        std::mt19937 generator;
        std::seed_seq seedSeq{static_cast<unsigned>(*pos), static_cast<unsigned>((buffer.getNumSamples()) * totalNumInputChannels * 2)};
        std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer (channel);
            float randomValue = 0.0f;
            auto* rp = buffer.getReadPointer(channel);

            for (int sample = 0; sample < buffer.getNumSamples(); sample++)
            {
                futureSamples[channel][(writePtr + sample) % (latencySamples+1)] = rp[sample];

                if ((*pos + sample) % blockSize == 0) {
                    randomValue = distribution(generator);
                    previousInvert = invert;
                    invert = randomValue >= probability;
                    if (previousInvert != invert) {
                        state = State::SMOOTH;
                        smoothCounter = 0;
                        smoothTargetSamples = blockSize * smooth;
                        if (invert) {
                            smoothStartValue = futureSamples[channel][(writePtr + sample + 1) % (latencySamples + 1)];
                            smoothEndValue = -futureSamples[channel][(writePtr + sample + smoothTargetSamples + 1) % (latencySamples + 1)];
                        } else {
                            smoothStartValue = -futureSamples[channel][(writePtr + sample + 1) % (latencySamples + 1)];
                            smoothEndValue = futureSamples[channel][(writePtr + sample + smoothTargetSamples + 1) % (latencySamples + 1)];
                        }
                    }
                }

                if (smoothCounter >= smoothTargetSamples) {
                    state = State::NORMAL;
                }

                float processedSample = 0.0f;

                if (state == State::NORMAL) {
                    processedSample = invert ? -(futureSamples[channel][(writePtr + sample + 1) % (latencySamples+1)])
                                                 : futureSamples[channel][(writePtr + sample + 1) % (latencySamples+1)];
                } else {
                    float ratio = (float)smoothCounter / (float)smoothTargetSamples;
                    float linearInterpolation = smoothStartValue * ratio + smoothEndValue * (1 - ratio);

                    float positionRadius = PI * (float)smoothCounter / (float)smoothTargetSamples;
                    long double ratioSin = std::cos(positionRadius) * -0.5L + 0.5L;

                    float sinInterpolation = smoothStartValue * ratioSin + smoothEndValue * (1 - ratioSin);

                    processedSample = sinInterpolation * smoothMode + (linearInterpolation) * (1.0 - smoothMode);

                    ++smoothCounter;
                }

                if (!bypass) {
                    long double p = mix * PI / 2;
                    channelData[sample] = std::sin(p) * processedSample + std::cos(p) * channelData[sample];
                } else {
                    channelData[sample] = futureSamples[channel][(writePtr + sample + 1) % (latencySamples+1)];
                }
            }
        }
        writePtr += buffer.getNumSamples();
        writePtr %= (latencySamples + 1);
    }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
    // return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
