#include "filter.h"
#include <cmath>
#include <cstdint>

LowPassFilter::LowPassFilter() {
    updateFilter();
}

void LowPassFilter::setSampleRate(double sampleRate) {
    m_sampleRate = sampleRate;
    updateFilter();
}

void LowPassFilter::setFrequency(double frequency) {
    m_frequency = frequency;
    updateFilter();
}

void LowPassFilter::reset() {
    m_prevInput = 0.0;
    m_prevOutput = 0.0;
}

void LowPassFilter::process(float* input, float* output, uint32_t numSamples) {
    // Simple first-order lowpass filter
    for (uint32_t i = 0; i < numSamples; ++i) {
        double input_d = static_cast<double>(input[i]);
        m_prevOutput = m_alpha * input_d + (1.0 - m_alpha) * m_prevOutput;
        output[i] = static_cast<float>(m_prevOutput);
    }
}

void LowPassFilter::updateFilter() {
    // Update simple filter coefficient
    double omega = 2.0 * M_PI * m_frequency / m_sampleRate;
    m_alpha = omega / (omega + 1.0);
}