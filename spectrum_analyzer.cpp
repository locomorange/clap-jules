#include "spectrum_analyzer.h"
#include <algorithm>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>

namespace clap_jules {

// Simple FFT Implementation
SimpleFFT::SimpleFFT(size_t size) : fft_size_(size) {
    // Ensure size is power of 2 for simple FFT
    size_t power_of_2 = 1;
    while (power_of_2 < size) {
        power_of_2 <<= 1;
    }
    fft_size_ = power_of_2;
    
    temp_buffer_.resize(fft_size_);
}

void SimpleFFT::computeSpectrum(const float* audio_data, std::vector<float>& spectrum) {
    // Copy audio data to complex buffer
    for (size_t i = 0; i < fft_size_; ++i) {
        temp_buffer_[i] = std::complex<float>(audio_data[i], 0.0f);
    }
    
    // Perform FFT
    fft(temp_buffer_);
    
    // Calculate magnitudes (only positive frequencies)
    size_t bin_count = fft_size_ / 2;
    spectrum.resize(bin_count);
    
    for (size_t i = 0; i < bin_count; ++i) {
        float real = temp_buffer_[i].real();
        float imag = temp_buffer_[i].imag();
        spectrum[i] = sqrt(real * real + imag * imag);
    }
}

void SimpleFFT::fft(std::vector<std::complex<float>>& data) {
    size_t n = data.size();
    
    // Bit-reverse the data
    bit_reverse(data);
    
    // Cooley-Tukey FFT
    for (size_t step = 2; step <= n; step <<= 1) {
        float angle = -2.0f * M_PI / step;
        std::complex<float> wn(cos(angle), sin(angle));
        
        for (size_t i = 0; i < n; i += step) {
            std::complex<float> w(1.0f, 0.0f);
            
            for (size_t j = 0; j < step / 2; ++j) {
                std::complex<float> u = data[i + j];
                std::complex<float> v = data[i + j + step / 2] * w;
                
                data[i + j] = u + v;
                data[i + j + step / 2] = u - v;
                
                w *= wn;
            }
        }
    }
}

void SimpleFFT::bit_reverse(std::vector<std::complex<float>>& data) {
    size_t n = data.size();
    size_t j = 0;
    
    for (size_t i = 1; i < n; ++i) {
        size_t bit = n >> 1;
        
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        
        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }
}

// Spectrum Analyzer Implementation
SpectrumAnalyzer::SpectrumAnalyzer(size_t fft_size, double sample_rate) 
    : sample_rate_(sample_rate), draw_mode_(SpectrumDrawMode::Lines), 
      buffer_write_pos_(0), smoothing_factor_(0.8f) {
    
    fft_ = std::make_unique<SimpleFFT>(fft_size);
    
    // Initialize buffers
    audio_buffer_.resize(fft_->getSize(), 0.0f);
    spectrum_magnitudes_.resize(fft_->getBinCount(), 0.0f);
    smoothed_spectrum_.resize(fft_->getBinCount(), 0.0f);
}

void SpectrumAnalyzer::processAudio(const float* audio_data, size_t frame_count) {
    // Add new audio data to circular buffer
    for (size_t i = 0; i < frame_count; ++i) {
        audio_buffer_[buffer_write_pos_] = audio_data[i];
        buffer_write_pos_ = (buffer_write_pos_ + 1) % audio_buffer_.size();
    }
    
    // Update spectrum analysis when we have enough data
    updateSpectrum();
}

void SpectrumAnalyzer::updateSpectrum() {
    // Perform FFT on current audio buffer
    fft_->computeSpectrum(audio_buffer_.data(), spectrum_magnitudes_);
    
    // Apply smoothing for visual stability
    for (size_t i = 0; i < spectrum_magnitudes_.size(); ++i) {
        // Convert to dB scale and apply smoothing
        float magnitude_db = 20.0f * log10f(std::max(spectrum_magnitudes_[i], 1e-6f));
        
        // Normalize to 0-1 range (assuming -60dB to 0dB range)
        magnitude_db = std::max(-60.0f, std::min(0.0f, magnitude_db));
        float normalized = (magnitude_db + 60.0f) / 60.0f;
        
        // Apply exponential smoothing
        smoothed_spectrum_[i] = smoothing_factor_ * smoothed_spectrum_[i] + 
                               (1.0f - smoothing_factor_) * normalized;
    }
    
    // Copy smoothed values back to spectrum_magnitudes_ for rendering
    spectrum_magnitudes_ = smoothed_spectrum_;
}

float SpectrumAnalyzer::getFrequencyForBin(size_t bin) const {
    return (float)bin * (sample_rate_ / 2.0) / fft_->getBinCount();
}

} // namespace clap_jules