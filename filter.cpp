#include "filter.h"
#include <kfr/dsp/biquad.hpp>
#include <kfr/dsp/samplerate.hpp> // For actual_samplerate if needed, or other utilities
#include <kfr/io/logging.hpp>   // For KFR logging if desired

LowPassFilter::LowPassFilter() : m_sampleRate(44100.0), m_frequency(1000.0) {
    // KFR_INFO << "LowPassFilter created."; // Example KFR logging
    updateCoeffs();
}

void LowPassFilter::setSampleRate(double sampleRate) {
    if (sampleRate <= 0) {
        // KFR_ERR << "Invalid sample rate: " << sampleRate;
        return;
    }
    m_sampleRate = sampleRate;
    updateCoeffs();
    m_filter.reset(); // Reset filter state when sample rate changes
}

void LowPassFilter::setFrequency(double frequency) {
    if (frequency <= 0) {
        // KFR_WARN << "Invalid frequency: " << frequency << ", clamping to a small positive value.";
        frequency = 1.0; // Avoid issues with zero or negative frequencies
    }
    if (frequency >= m_sampleRate / 2.0) {
        // KFR_WARN << "Frequency " << frequency << " is too high for sample rate " << m_sampleRate << ", clamping to Nyquist.";
        frequency = m_sampleRate / 2.0 - 1.0; // Clamp just below Nyquist
        if (frequency <= 0) frequency = 1.0; // Ensure positive
    }
    m_frequency = frequency;
    updateCoeffs();
}

void LowPassFilter::reset() {
    m_filter.reset();
    // KFR_INFO << "LowPassFilter reset.";
}

void LowPassFilter::process(float* input, float* output, uint32_t numSamples) {
    if (numSamples == 0) {
        return;
    }

    // Ensure temporary buffer is large enough
    if (m_tempBuffer.size() < numSamples) {
        m_tempBuffer.resize(numSamples);
    }

    // Copy input to KFR univector
    // In a real-world scenario, you might want to avoid this copy if possible,
    // or use kfr::temparray_handler for more advanced buffer management.
    for (uint32_t i = 0; i < numSamples; ++i) {
        m_tempBuffer[i] = input[i];
    }

    // Apply the filter
    m_filter.apply(m_tempBuffer);

    // Copy result from KFR univector to output
    for (uint32_t i = 0; i < numSamples; ++i) {
        output[i] = m_tempBuffer[i];
    }
}

void LowPassFilter::updateCoeffs() {
    // KFR_INFO << "Updating LowPassFilter coeffs: Freq=" << m_frequency << " Hz, SR=" << m_sampleRate << " Hz";
    // Use KFR to generate second-order Linkwitz-Riley lowpass coefficients
    // A Butterworth filter is also a good choice: kfr::butterworth_lowpass<kfr::f32>(kfr::to_normalized_freq(m_frequency, m_sampleRate), 2 /*order*/)
    // For a simple 2nd order lowpass, Linkwitz-Riley (Butterworth squared) is often used.
    // Or simply a Butterworth of order 2. Let's use Butterworth order 2 for simplicity.
    // The Q factor for Butterworth is 1/sqrt(2) ~= 0.707
    m_coeffs = kfr::biquad_lowpass_coeffs<kfr::f32>(kfr::to_normalized_freq(m_frequency, m_sampleRate), 0.70710678118); // Q = 1/sqrt(2) for Butterworth
    m_filter.set_coeffs(m_coeffs);
}