#pragma once

#include <kfr/all.hpp>
#include <boost/di.hpp>
#include <clap/clap.h>
#include <memory>
#include <atomic>
#include "common/dll_export.h"

namespace ClapeJules {

class CLAP_JULES_EXPORT AudioProcessor {
public:
    AudioProcessor();
    ~AudioProcessor();

    // Audio processing methods
    void setFrequency(float frequency);
    float getFrequency() const;
    void setSampleRate(double sampleRate);
    void process(const float* const* inputs, float* const* outputs, uint32_t frameCount, uint32_t channelCount);
    void reset();

private:
    // KFR IIR filter using modern API
    std::unique_ptr<kfr::iir_filter<float>> m_filter;
    std::atomic<float> m_frequency{1000.0f}; // Default 1kHz cutoff
    double m_sampleRate{44100.0};
    static constexpr float Q_FACTOR = 0.7071f; // Butterworth response
    
    void updateFilter();
};

} // namespace ClapeJules