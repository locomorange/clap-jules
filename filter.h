#pragma once
#include <cstdint>

class LowPassFilter {
public:
    LowPassFilter();
    ~LowPassFilter() = default;
    
    void setSampleRate(double sampleRate);
    void setFrequency(double frequency);
    void reset();
    void process(float* input, float* output, uint32_t numSamples);
    
private:
    double m_sampleRate = 44100.0;
    double m_frequency = 1000.0;
    
    // Simple implementation for now
    double m_prevInput = 0.0;
    double m_prevOutput = 0.0;
    double m_alpha = 0.5;
    
    void updateFilter();
};