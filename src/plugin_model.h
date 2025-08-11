#pragma once

#include <boost/di.hpp>
#include <memory>

namespace plugin {

// Model - contains plugin state and parameters
class PluginModel {
public:
    PluginModel() : sample_rate_(44100.0), cutoff_frequency_(1000.0) {}
    
    // Parameter setters
    void SetSampleRate(double sample_rate) { sample_rate_ = sample_rate; }
    void SetCutoffFrequency(double frequency) { cutoff_frequency_ = frequency; }
    
    // Parameter getters
    double GetSampleRate() const { return sample_rate_; }
    double GetCutoffFrequency() const { return cutoff_frequency_; }
    
private:
    double sample_rate_;
    double cutoff_frequency_;
};

} // namespace plugin