#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <complex>
#include <memory>

namespace clap_jules {

// FFT implementation for spectrum analysis
class SimpleFFT {
public:
    SimpleFFT(size_t size);
    
    // Perform FFT on input audio data and return magnitude spectrum
    void computeSpectrum(const float* audio_data, std::vector<float>& spectrum);
    
    size_t getSize() const { return fft_size_; }
    size_t getBinCount() const { return fft_size_ / 2; } // Only positive frequencies
    
private:
    void fft(std::vector<std::complex<float>>& data);
    void bit_reverse(std::vector<std::complex<float>>& data);
    
    size_t fft_size_;
    std::vector<std::complex<float>> temp_buffer_;
};

// Spectrum analyzer drawing modes
enum class SpectrumDrawMode {
    Lines = 0,
    Dots,
    Bins, 
    Fill
};

// Spectrum analyzer state and functionality
class SpectrumAnalyzer {
public:
    SpectrumAnalyzer(size_t fft_size = 512, double sample_rate = 44100.0);
    
    // Process new audio data 
    void processAudio(const float* audio_data, size_t frame_count);
    
    // Get current spectrum data for rendering
    const std::vector<float>& getSpectrum() const { return spectrum_magnitudes_; }
    
    // Drawing mode control
    void setDrawMode(SpectrumDrawMode mode) { draw_mode_ = mode; }
    SpectrumDrawMode getDrawMode() const { return draw_mode_; }
    
    // Get frequency for a given bin
    float getFrequencyForBin(size_t bin) const;
    
    // Get parameters
    size_t getBinCount() const { return fft_->getBinCount(); }
    size_t getFFTSize() const { return fft_->getSize(); }
    
private:
    void updateSpectrum();
    
    std::unique_ptr<SimpleFFT> fft_;
    double sample_rate_;
    SpectrumDrawMode draw_mode_;
    
    // Audio buffering for FFT
    std::vector<float> audio_buffer_;
    size_t buffer_write_pos_;
    
    // Spectrum data
    std::vector<float> spectrum_magnitudes_;
    
    // Smoothing for visual stability
    std::vector<float> smoothed_spectrum_;
    float smoothing_factor_;
};

} // namespace clap_jules