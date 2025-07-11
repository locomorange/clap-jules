#pragma once
#include <cstdint>
#include <kfr/all.hpp> // KFR main header

class LowPassFilter {
public:
    LowPassFilter();
    ~LowPassFilter() = default;
    
    void setSampleRate(double sampleRate);
    void setFrequency(double frequency);
    void reset();
    // Process a single channel of audio
    void process(float* input, float* output, uint32_t numSamples);
    
private:
    double m_sampleRate;
    double m_frequency;
    
    kfr::biquad_filter<kfr::f32> m_filter;
    kfr::biquad_coeffs<kfr::f32> m_coeffs;
    kfr::univector<kfr::f32> m_tempBuffer; // Temporary buffer for KFR processing

    void updateCoeffs();
};