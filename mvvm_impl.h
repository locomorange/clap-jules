#include "mvvm.h"
#include "filter.h"
#include <memory>
#include <vector>

// Concrete implementation of AudioModel
class FilterAudioModel : public AudioModel {
public:
    FilterAudioModel() : m_frequency(1000.0) {
        // Initialize filters for stereo
        m_filters.resize(2);
    }
    
    void setFilterFrequency(double frequency) override {
        m_frequency = frequency;
        for (auto& filter : m_filters) {
            filter.setFrequency(frequency);
        }
    }
    
    double getFilterFrequency() const override {
        return m_frequency;
    }
    
    void processAudio(float* input, float* output, uint32_t numSamples, uint32_t channels) override {
        // Ensure we have enough filters for the channel count
        if (m_filters.size() < channels) {
            m_filters.resize(channels);
            for (size_t i = 0; i < channels; ++i) {
                m_filters[i].setSampleRate(m_sampleRate);
                m_filters[i].setFrequency(m_frequency);
            }
        }
        
        // Process each channel
        for (uint32_t ch = 0; ch < channels; ++ch) {
            // Extract channel data
            std::vector<float> channelInput(numSamples);
            std::vector<float> channelOutput(numSamples);
            
            for (uint32_t i = 0; i < numSamples; ++i) {
                channelInput[i] = input[i * channels + ch];
            }
            
            // Process through filter
            m_filters[ch].process(channelInput.data(), channelOutput.data(), numSamples);
            
            // Interleave back to output
            for (uint32_t i = 0; i < numSamples; ++i) {
                output[i * channels + ch] = channelOutput[i];
            }
        }
    }
    
    void setSampleRate(double sampleRate) override {
        m_sampleRate = sampleRate;
        for (auto& filter : m_filters) {
            filter.setSampleRate(sampleRate);
        }
    }
    
    void reset() override {
        for (auto& filter : m_filters) {
            filter.reset();
        }
    }
    
private:
    double m_frequency;
    double m_sampleRate = 44100.0;
    std::vector<LowPassFilter> m_filters;
};

// Concrete implementation of PluginViewModel
class FilterPluginViewModel : public PluginViewModel {
public:
    FilterPluginViewModel(std::shared_ptr<AudioModel> model) : m_model(model) {}
    
    void setFrequency(double frequency) override {
        m_model->setFilterFrequency(frequency);
    }
    
    double getFrequency() const override {
        return m_model->getFilterFrequency();
    }
    
    void processAudio(float* input, float* output, uint32_t numSamples, uint32_t channels) override {
        m_model->processAudio(input, output, numSamples, channels);
    }
    
    void setSampleRate(double sampleRate) override {
        m_model->setSampleRate(sampleRate);
    }
    
    void reset() override {
        m_model->reset();
    }
    
private:
    std::shared_ptr<AudioModel> m_model;
};