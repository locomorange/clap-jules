#include "spectrum_analyzer.h"
#include <cstring>
#include <cmath>

SpectrumAnalyzer::SpectrumAnalyzer()
    : sample_rate_(44100.0)
    , input_buffer_pos_(0)
    , has_new_data_(false)
    , smoothing_factor_(0.7f)
{
}

void SpectrumAnalyzer::initialize(double sample_rate) {
    sample_rate_ = sample_rate;
    input_buffer_pos_ = 0;
    has_new_data_ = false;
    
    // Initialize buffers
    input_buffer_.resize(FFT_SIZE, 0.0f);
    windowing_function_.resize(FFT_SIZE);
    fft_buffer_.resize(FFT_SIZE);
    magnitude_spectrum_.resize(NUM_BINS, 0.0f);
    display_spectrum_.resize(DISPLAY_BINS, 0.0f);
    previous_spectrum_.resize(DISPLAY_BINS, 0.0f);
    frequency_bins_.resize(DISPLAY_BINS);
    
    // Create Hann window
    for (size_t i = 0; i < FFT_SIZE; ++i) {
        windowing_function_[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (FFT_SIZE - 1)));
    }
    
    // Create logarithmic frequency bins
    create_logarithmic_bins();
}

void SpectrumAnalyzer::process_samples(const float* samples, size_t num_samples) {
    for (size_t i = 0; i < num_samples; ++i) {
        input_buffer_[input_buffer_pos_] = samples[i];
        input_buffer_pos_ = (input_buffer_pos_ + 1) % FFT_SIZE;
        
        // Process FFT when we have enough samples
        if (input_buffer_pos_ % HOP_SIZE == 0) {
            perform_fft();
            convert_to_magnitude_spectrum();
            update_display_spectrum();
            has_new_data_ = true;
        }
    }
}

void SpectrumAnalyzer::perform_fft() {
    // Copy input buffer to FFT buffer with windowing
    for (size_t i = 0; i < FFT_SIZE; ++i) {
        size_t buffer_idx = (input_buffer_pos_ + i) % FFT_SIZE;
        fft_buffer_[i] = std::complex<float>(input_buffer_[buffer_idx] * windowing_function_[i], 0.0f);
    }
    
    // Perform FFT
    fft_radix2(fft_buffer_);
}

void SpectrumAnalyzer::convert_to_magnitude_spectrum() {
    const float scale = 2.0f / FFT_SIZE;
    
    for (size_t i = 0; i < NUM_BINS; ++i) {
        float magnitude = std::abs(fft_buffer_[i]) * scale;
        
        // Convert to dB with floor
        magnitude_spectrum_[i] = magnitude > 1e-6f ? 
            20.0f * std::log10(magnitude) : -120.0f;
    }
}

void SpectrumAnalyzer::create_logarithmic_bins() {
    const float log_min = std::log10(MIN_FREQ);
    const float log_max = std::log10(MAX_FREQ);
    const float log_range = log_max - log_min;
    
    for (size_t i = 0; i < DISPLAY_BINS; ++i) {
        float log_freq = log_min + (log_range * i) / (DISPLAY_BINS - 1);
        frequency_bins_[i] = std::pow(10.0f, log_freq);
    }
}

void SpectrumAnalyzer::update_display_spectrum() {
    const float freq_per_bin = static_cast<float>(sample_rate_) / (2.0f * FFT_SIZE);
    
    for (size_t i = 0; i < DISPLAY_BINS; ++i) {
        float target_freq = frequency_bins_[i];
        size_t fft_bin = static_cast<size_t>(target_freq / freq_per_bin);
        
        if (fft_bin < NUM_BINS) {
            float current_value = magnitude_spectrum_[fft_bin];
            
            // Apply smoothing
            display_spectrum_[i] = smoothing_factor_ * previous_spectrum_[i] + 
                                   (1.0f - smoothing_factor_) * current_value;
            
            previous_spectrum_[i] = display_spectrum_[i];
        }
    }
}

void SpectrumAnalyzer::fft_radix2(std::vector<std::complex<float>>& data) {
    const size_t N = data.size();
    
    // Bit-reverse permutation
    for (size_t i = 1, j = 0; i < N; ++i) {
        size_t bit = N >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }
    
    // FFT computation
    for (size_t length = 2; length <= N; length <<= 1) {
        float angle = -2.0f * M_PI / length;
        std::complex<float> wlen(std::cos(angle), std::sin(angle));
        
        for (size_t i = 0; i < N; i += length) {
            std::complex<float> w(1.0f, 0.0f);
            
            for (size_t j = 0; j < length / 2; ++j) {
                std::complex<float> u = data[i + j];
                std::complex<float> v = data[i + j + length / 2] * w;
                
                data[i + j] = u + v;
                data[i + j + length / 2] = u - v;
                w *= wlen;
            }
        }
    }
}