#pragma once

#include "fft.h"
#include "graphics/skia_graphics.h"
#include <vector>
#include <atomic>
#include <mutex>
#include <memory>

namespace clap_jules {
namespace audio {

/**
 * Visualization types for the spectrum analyzer
 */
enum class SpectrumVisualizationType {
    Lines,
    Dots, 
    Bins,
    Fill
};

/**
 * Real-time spectrum analyzer that processes audio input and provides
 * visualization data for different rendering styles
 */
class SpectrumAnalyzer {
public:
    static constexpr size_t DEFAULT_FFT_SIZE = 1024;
    static constexpr size_t MAX_FFT_SIZE = 4096;
    static constexpr float MIN_DB = -60.0f;
    static constexpr float MAX_DB = 0.0f;
    
    SpectrumAnalyzer(size_t fft_size = DEFAULT_FFT_SIZE, float sample_rate = 44100.0f);
    ~SpectrumAnalyzer() = default;
    
    /**
     * Process audio samples and update spectrum data
     * @param samples Input audio samples
     * @param num_samples Number of samples to process
     */
    void processAudio(const float* samples, size_t num_samples);
    
    /**
     * Set the visualization type
     */
    void setVisualizationType(SpectrumVisualizationType type) {
        visualization_type_ = type;
    }
    
    /**
     * Get current visualization type
     */
    SpectrumVisualizationType getVisualizationType() const {
        return visualization_type_;
    }
    
    /**
     * Set sample rate (affects frequency scale)
     */
    void setSampleRate(float sample_rate) {
        sample_rate_ = sample_rate;
    }
    
    /**
     * Get the current magnitude spectrum (thread-safe)
     * @param output Output vector to fill with magnitude data
     * @return true if new data is available
     */
    bool getMagnitudeSpectrum(std::vector<float>& output);
    
    /**
     * Get frequency bin for a given index
     * @param bin_index Index of the frequency bin
     * @return Frequency in Hz
     */
    float getFrequencyForBin(size_t bin_index) const;
    
    /**  
     * Convert magnitude to decibels
     */
    static float magnitudeToDb(float magnitude);
    
    /**
     * Get the number of frequency bins
     */
    size_t getNumBins() const { return fft_size_ / 2; }
    
    /**
     * Render the spectrum using the specified graphics context
     * @param graphics Graphics context for rendering
     * @param bounds Rendering bounds
     */
    void render(graphics::GraphicsContext* graphics, const graphics::Rect& bounds);
    
private:
    size_t fft_size_;
    float sample_rate_;
    SpectrumVisualizationType visualization_type_;
    
    // Audio processing
    std::vector<float> input_buffer_;
    std::vector<float> windowed_buffer_;
    std::vector<FFT::Complex> fft_output_;
    std::vector<float> magnitude_spectrum_;
    std::vector<float> smoothed_spectrum_;
    
    // Thread safety
    mutable std::mutex spectrum_mutex_;
    std::atomic<bool> new_data_available_;
    
    // Rendering state
    float smoothing_factor_;
    
    // Internal methods
    void performFFT();
    void smoothSpectrum();
    
    // Rendering methods for different visualization types
    void renderLines(graphics::GraphicsContext* graphics, const graphics::Rect& bounds,
                    const std::vector<float>& spectrum);
    void renderDots(graphics::GraphicsContext* graphics, const graphics::Rect& bounds,
                   const std::vector<float>& spectrum);
    void renderBins(graphics::GraphicsContext* graphics, const graphics::Rect& bounds,
                   const std::vector<float>& spectrum);
    void renderFill(graphics::GraphicsContext* graphics, const graphics::Rect& bounds,
                   const std::vector<float>& spectrum);
                   
    // Utility methods
    float mapDbToY(float db, const graphics::Rect& bounds) const;
    float mapBinToX(size_t bin, const graphics::Rect& bounds) const;
    graphics::Color getSpectrumColor(float normalized_magnitude) const;
};

} // namespace audio
} // namespace clap_jules