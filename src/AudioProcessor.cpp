#include "AudioProcessor.h"
#include <algorithm>
#include <cmath>

using namespace kfr;

namespace ClapeJules {

AudioProcessor::AudioProcessor() {
    updateFilter();
}

AudioProcessor::~AudioProcessor() = default;

void AudioProcessor::setFrequency(float frequency) {
    // Clamp frequency to valid range (20Hz to Nyquist)
    const float maxFreq = static_cast<float>(m_sampleRate * 0.45); // Leave some headroom
    frequency = std::clamp(frequency, 20.0f, maxFreq);
    
    m_frequency.store(frequency);
    updateFilter();
}

float AudioProcessor::getFrequency() const {
    return m_frequency.load();
}

void AudioProcessor::setSampleRate(double sampleRate) {
    m_sampleRate = sampleRate;
    updateFilter();
}

void AudioProcessor::process(const float* const* inputs, float* const* outputs, uint32_t frameCount, uint32_t channelCount) {
    if (!m_filter || !inputs || !outputs || frameCount == 0 || channelCount == 0) {
        return;
    }

    // Process each channel
    for (uint32_t channel = 0; channel < channelCount; ++channel) {
        if (inputs[channel] && outputs[channel]) {
            // Copy input to output first
            std::copy(inputs[channel], inputs[channel] + frameCount, outputs[channel]);
            
            // Create univector view and apply filter in-place
            auto buffer = make_univector(outputs[channel], frameCount);
            m_filter->apply(buffer);
        }
    }
}

void AudioProcessor::reset() {
    if (m_filter) {
        m_filter->reset();
    }
}

void AudioProcessor::updateFilter() {
    // Create new lowpass filter with current frequency using modern KFR API
    const float normalizedFreq = m_frequency.load() / static_cast<float>(m_sampleRate);
    auto section = biquad_lowpass<float>(normalizedFreq, Q_FACTOR);
    
    // Use iir_params for construction
    m_filter = std::make_unique<iir_filter<float>>(iir_params{section});
}

} // namespace ClapeJules