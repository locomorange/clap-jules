#pragma once

#include <vector>
#include <memory>

namespace audio {

struct AudioBuffer {
    float** channels;
    uint32_t frameCount;
    uint32_t channelCount;
    
    AudioBuffer(uint32_t frames, uint32_t numChannels)
        : frameCount(frames), channelCount(numChannels) {
        channelData.resize(numChannels);
        data.resize(numChannels);
        for (uint32_t i = 0; i < numChannels; ++i) {
            channelData[i].resize(frames);
            data[i] = channelData[i].data();
        }
        channels = data.data();
    }
    
private:
    std::vector<std::vector<float>> channelData;
    std::vector<float*> data;
};

class IAudioProcessor {
public:
    virtual ~IAudioProcessor() = default;
    virtual void process(AudioBuffer& buffer) = 0;
    virtual void setSampleRate(double sampleRate) = 0;
    virtual void setBufferSize(uint32_t bufferSize) = 0;
    virtual void reset() = 0;
};

class AudioProcessor : public IAudioProcessor {
public:
    AudioProcessor() = default;
    virtual ~AudioProcessor() = default;
    
    void process(AudioBuffer& buffer) override {
        // Default implementation - pass through
        // Derived classes should override this
    }
    
    void setSampleRate(double sampleRate) override {
        sampleRate_ = sampleRate;
    }
    
    void setBufferSize(uint32_t bufferSize) override {
        bufferSize_ = bufferSize;
    }
    
    void reset() override {
        // Default implementation - do nothing
        // Derived classes should override this
    }
    
protected:
    double sampleRate_ = 44100.0;
    uint32_t bufferSize_ = 512;
};

}