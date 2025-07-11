#pragma once

#ifdef HAVE_BOOST_DI
#include <boost/di.hpp>
#endif

// Audio processing model
class AudioModel {
public:
    virtual ~AudioModel() = default;
    virtual void setFilterFrequency(double frequency) = 0;
    virtual double getFilterFrequency() const = 0;
    virtual void processAudio(float* input, float* output, uint32_t numSamples, uint32_t channels) = 0;
    virtual void setSampleRate(double sampleRate) = 0;
    virtual void reset() = 0;
};

// View model for MVVM pattern
class PluginViewModel {
public:
    virtual ~PluginViewModel() = default;
    virtual void setFrequency(double frequency) = 0;
    virtual double getFrequency() const = 0;
    virtual void processAudio(float* input, float* output, uint32_t numSamples, uint32_t channels) = 0;
    virtual void setSampleRate(double sampleRate) = 0;
    virtual void reset() = 0;
};

// View interface (for future GUI implementation)
class PluginView {
public:
    virtual ~PluginView() = default;
    virtual void updateFrequencyDisplay(double frequency) = 0;
    virtual void setViewModel(PluginViewModel* viewModel) = 0;
};