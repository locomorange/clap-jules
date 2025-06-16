#include "spectrum_analyzer.h"
#include <algorithm>
#include <cstring>
#include <cmath>

namespace clap_jules {
namespace audio {

SpectrumAnalyzer::SpectrumAnalyzer(size_t fft_size, float sample_rate)
    : fft_size_(std::min(fft_size, MAX_FFT_SIZE))
    , sample_rate_(sample_rate)
    , visualization_type_(SpectrumVisualizationType::Lines)
    , new_data_available_(false)
    , smoothing_factor_(0.8f)
{
    // Ensure FFT size is power of 2
    fft_size_ = FFT::nextPowerOf2(fft_size_);
    
    // Initialize buffers
    input_buffer_.reserve(fft_size_);
    windowed_buffer_.resize(fft_size_);
    fft_output_.resize(fft_size_);
    magnitude_spectrum_.resize(fft_size_ / 2);
    smoothed_spectrum_.resize(fft_size_ / 2);
    
    // Initialize smoothed spectrum to minimum value
    std::fill(smoothed_spectrum_.begin(), smoothed_spectrum_.end(), MIN_DB);
}

void SpectrumAnalyzer::processAudio(const float* samples, size_t num_samples) {
    if (!samples || num_samples == 0) {
        return;
    }
    
    // Add samples to input buffer
    for (size_t i = 0; i < num_samples; ++i) {
        input_buffer_.push_back(samples[i]);
        
        // When we have enough samples, perform FFT
        if (input_buffer_.size() >= fft_size_) {
            performFFT();
            
            // Remove processed samples (with overlap)
            size_t overlap = fft_size_ / 4;
            input_buffer_.erase(input_buffer_.begin(), input_buffer_.begin() + (fft_size_ - overlap));
        }
    }
}

void SpectrumAnalyzer::performFFT() {
    // Copy and pad input buffer
    std::vector<float> padded_input(fft_size_, 0.0f);
    size_t copy_size = std::min(input_buffer_.size(), fft_size_);
    std::copy(input_buffer_.begin(), input_buffer_.begin() + copy_size, padded_input.begin());
    
    // Apply windowing
    FFT::applyHannWindow(padded_input, windowed_buffer_);
    
    // Perform FFT
    FFT::forward(windowed_buffer_, fft_output_, fft_size_);
    
    // Convert to magnitude spectrum
    std::vector<float> raw_magnitude;
    FFT::magnitude(fft_output_, raw_magnitude);
    
    // Convert to dB and store
    {
        std::lock_guard<std::mutex> lock(spectrum_mutex_);
        
        for (size_t i = 0; i < magnitude_spectrum_.size(); ++i) {
            magnitude_spectrum_[i] = magnitudeToDb(raw_magnitude[i]);
        }
        
        smoothSpectrum();
        new_data_available_.store(true);
    }
}

void SpectrumAnalyzer::smoothSpectrum() {
    for (size_t i = 0; i < smoothed_spectrum_.size(); ++i) {
        smoothed_spectrum_[i] = smoothing_factor_ * smoothed_spectrum_[i] + 
                               (1.0f - smoothing_factor_) * magnitude_spectrum_[i];
    }
}

bool SpectrumAnalyzer::getMagnitudeSpectrum(std::vector<float>& output) {
    std::lock_guard<std::mutex> lock(spectrum_mutex_);
    
    if (!new_data_available_.load()) {
        return false;
    }
    
    output = smoothed_spectrum_;
    new_data_available_.store(false);
    return true;
}

float SpectrumAnalyzer::getFrequencyForBin(size_t bin_index) const {
    if (bin_index >= getNumBins()) {
        return 0.0f;
    }
    
    return (bin_index * sample_rate_) / (2.0f * getNumBins());
}

float SpectrumAnalyzer::magnitudeToDb(float magnitude) {
    if (magnitude <= 0.0f) {
        return MIN_DB;
    }
    
    float db = 20.0f * log10f(magnitude);
    return std::max(db, MIN_DB);
}

void SpectrumAnalyzer::render(graphics::GraphicsContext* graphics, const graphics::Rect& bounds) {
    if (!graphics) {
        return;
    }
    
    std::vector<float> spectrum;
    if (!getMagnitudeSpectrum(spectrum)) {
        return; // No new data to render
    }
    
    // Clear background
    graphics->save();
    
    // Render based on visualization type
    switch (visualization_type_) {
        case SpectrumVisualizationType::Lines:
            renderLines(graphics, bounds, spectrum);
            break;
        case SpectrumVisualizationType::Dots:
            renderDots(graphics, bounds, spectrum);
            break;
        case SpectrumVisualizationType::Bins:
            renderBins(graphics, bounds, spectrum);
            break;
        case SpectrumVisualizationType::Fill:
            renderFill(graphics, bounds, spectrum);
            break;
    }
    
    graphics->restore();
}

void SpectrumAnalyzer::renderLines(graphics::GraphicsContext* graphics, const graphics::Rect& bounds,
                                  const std::vector<float>& spectrum) {
    if (spectrum.size() < 2) return;
    
    graphics::Point prev_point(mapBinToX(0, bounds), mapDbToY(spectrum[0], bounds));
    
    for (size_t i = 1; i < spectrum.size(); ++i) {
        graphics::Point curr_point(mapBinToX(i, bounds), mapDbToY(spectrum[i], bounds));
        
        // Color based on magnitude
        float normalized = (spectrum[i] - MIN_DB) / (MAX_DB - MIN_DB);
        graphics::Color color = getSpectrumColor(normalized);
        
        graphics->drawLine(prev_point, curr_point, color, 1.5f);
        prev_point = curr_point;
    }
}

void SpectrumAnalyzer::renderDots(graphics::GraphicsContext* graphics, const graphics::Rect& bounds,
                                 const std::vector<float>& spectrum) {
    for (size_t i = 0; i < spectrum.size(); ++i) {
        float x = mapBinToX(i, bounds);
        float y = mapDbToY(spectrum[i], bounds);
        
        float normalized = (spectrum[i] - MIN_DB) / (MAX_DB - MIN_DB);
        graphics::Color color = getSpectrumColor(normalized);
        
        graphics->drawCircle(graphics::Point(x, y), 1.5f, color);
    }
}

void SpectrumAnalyzer::renderBins(graphics::GraphicsContext* graphics, const graphics::Rect& bounds,
                                 const std::vector<float>& spectrum) {
    float bin_width = bounds.width / spectrum.size();
    
    for (size_t i = 0; i < spectrum.size(); ++i) {
        float x = bounds.x + i * bin_width;
        float y = mapDbToY(spectrum[i], bounds);
        float height = bounds.y + bounds.height - y;
        
        float normalized = (spectrum[i] - MIN_DB) / (MAX_DB - MIN_DB);
        graphics::Color color = getSpectrumColor(normalized);
        
        graphics->drawRect(graphics::Rect(x, y, bin_width - 1, height), color);
    }
}

void SpectrumAnalyzer::renderFill(graphics::GraphicsContext* graphics, const graphics::Rect& bounds,
                                 const std::vector<float>& spectrum) {
    if (spectrum.size() < 2) return;
    
    // Draw filled area using rectangles for now (could be optimized with polygon fill)
    float bin_width = bounds.width / spectrum.size();
    
    for (size_t i = 0; i < spectrum.size(); ++i) {
        float x = bounds.x + i * bin_width;
        float y = mapDbToY(spectrum[i], bounds);
        float height = bounds.y + bounds.height - y;
        
        float normalized = (spectrum[i] - MIN_DB) / (MAX_DB - MIN_DB);
        graphics::Color color = getSpectrumColor(normalized);
        
        // Make fill semi-transparent
        color.a = 128;
        graphics->drawRect(graphics::Rect(x, y, bin_width, height), color);
    }
}

float SpectrumAnalyzer::mapDbToY(float db, const graphics::Rect& bounds) const {
    float normalized = (db - MIN_DB) / (MAX_DB - MIN_DB);
    normalized = std::max(0.0f, std::min(1.0f, normalized));
    
    // Flip Y coordinate (higher magnitude = lower Y)
    return bounds.y + bounds.height * (1.0f - normalized);
}

float SpectrumAnalyzer::mapBinToX(size_t bin, const graphics::Rect& bounds) const {
    float normalized = static_cast<float>(bin) / getNumBins();
    return bounds.x + bounds.width * normalized;
}

graphics::Color SpectrumAnalyzer::getSpectrumColor(float normalized_magnitude) const {
    // Create color gradient from blue (low) to red (high)
    normalized_magnitude = std::max(0.0f, std::min(1.0f, normalized_magnitude));
    
    if (normalized_magnitude < 0.5f) {
        // Blue to cyan
        float t = normalized_magnitude * 2.0f;
        return graphics::Color(0, static_cast<uint8_t>(t * 255), 255, 255);
    } else {
        // Cyan to red
        float t = (normalized_magnitude - 0.5f) * 2.0f;
        return graphics::Color(static_cast<uint8_t>(t * 255), 255, static_cast<uint8_t>((1.0f - t) * 255), 255);
    }
}

} // namespace audio
} // namespace clap_jules