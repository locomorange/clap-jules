#pragma once

#include "plugin_model.h"
#include <kfr/all.hpp>
#include <memory>

namespace plugin {

// Audio processor using KFR for low-pass filtering
class AudioProcessor {
public:
    explicit AudioProcessor(std::shared_ptr<PluginModel> model) 
        : model_(model), filter_initialized_(false) {}
    
    void Initialize(double sample_rate) {
        model_->SetSampleRate(sample_rate);
        InitializeFilter();
    }
    
    void ProcessAudio(const float* input, float* output, uint32_t frames) {
        if (!filter_initialized_) {
            InitializeFilter();
        }
        
        // Simple lowpass filter using KFR
        for (uint32_t i = 0; i < frames; ++i) {
            output[i] = ApplyLowPassFilter(input[i]);
        }
    }
    
    void UpdateCutoffFrequency(double frequency) {
        model_->SetCutoffFrequency(frequency);
        InitializeFilter();
    }
    
private:
    void InitializeFilter() {
        double sample_rate = model_->GetSampleRate();
        double cutoff = model_->GetCutoffFrequency();
        
        // Simple one-pole lowpass filter coefficient
        // cutoff_normalized = cutoff / (sample_rate / 2)
        double cutoff_normalized = cutoff / (sample_rate * 0.5);
        cutoff_normalized = std::min(cutoff_normalized, 0.99); // Avoid instability
        
        // Simple IIR coefficient: y[n] = a * x[n] + (1-a) * y[n-1]
        alpha_ = cutoff_normalized;
        previous_output_ = 0.0f;
        filter_initialized_ = true;
    }
    
    float ApplyLowPassFilter(float input) {
        float output = alpha_ * input + (1.0f - alpha_) * previous_output_;
        previous_output_ = output;
        return output;
    }
    
    std::shared_ptr<PluginModel> model_;
    bool filter_initialized_;
    float alpha_;
    float previous_output_;
};

} // namespace plugin