#pragma once

#include <vector>
#include <complex>
#include <memory>
#include <cmath>
#include <algorithm>

/**
 * Real-time spectrum analyzer using FFT for frequency domain analysis
 * Optimized for audio plugin use with logarithmic frequency scaling
 */
class SpectrumAnalyzer {
public:
    static constexpr size_t FFT_SIZE = 2048;
    static constexpr size_t HOP_SIZE = FFT_SIZE / 4; // 75% overlap for smooth updates
    static constexpr size_t NUM_BINS = FFT_SIZE / 2;
    static constexpr float MIN_FREQ = 20.0f;
    static constexpr float MAX_FREQ = 20000.0f;
    static constexpr size_t DISPLAY_BINS = 256; // Number of bins for display
    
    SpectrumAnalyzer();
    ~SpectrumAnalyzer() = default;
    
    // Initialize with sample rate
    void initialize(double sample_rate);
    
    // Process audio samples (mono or average of stereo)
    void process_samples(const float* samples, size_t num_samples);
    
    // Get spectrum data for display (logarithmically spaced frequencies)
    const std::vector<float>& get_spectrum_data() const { return display_spectrum_; }
    
    // Get frequency values corresponding to display bins
    const std::vector<float>& get_frequency_bins() const { return frequency_bins_; }
    
    // Check if new spectrum data is available
    bool has_new_data() const { return has_new_data_; }
    void acknowledge_data() { has_new_data_ = false; }
    
    // Smoothing factor for spectrum display (0.0 = no smoothing, 0.9 = heavy smoothing)
    void set_smoothing(float smoothing) { smoothing_factor_ = std::clamp(smoothing, 0.0f, 0.99f); }

private:
    void perform_fft();
    void convert_to_magnitude_spectrum();
    void create_logarithmic_bins();
    void apply_windowing();
    void update_display_spectrum();
    
    // Simple radix-2 FFT implementation
    void fft_radix2(std::vector<std::complex<float>>& data);
    
    double sample_rate_;
    size_t input_buffer_pos_;
    size_t samples_since_last_fft_;
    size_t total_samples_processed_;
    bool has_new_data_;
    float smoothing_factor_;
    
    // Buffers
    std::vector<float> input_buffer_;
    std::vector<float> windowing_function_;
    std::vector<std::complex<float>> fft_buffer_;
    std::vector<float> magnitude_spectrum_;
    std::vector<float> display_spectrum_;
    std::vector<float> previous_spectrum_;
    std::vector<float> frequency_bins_;
};