#pragma once

#include <vector>
#include <complex>
#include <cmath>

namespace clap_jules {
namespace audio {

/**
 * Simple FFT implementation for spectrum analysis
 * Based on Cooley-Tukey algorithm
 */
class FFT {
public:
    using Complex = std::complex<float>;
    
    /**
     * Perform FFT on input data
     * @param input Input samples (real values)
     * @param output Output frequency domain data (complex values)
     * @param size Size of input/output (must be power of 2)
     */
    static void forward(const std::vector<float>& input, std::vector<Complex>& output, size_t size) {
        // Ensure size is power of 2
        if (size == 0 || (size & (size - 1)) != 0) {
            return;
        }
        
        output.resize(size);
        
        // Copy input to output (converting to complex)
        for (size_t i = 0; i < size; ++i) {
            output[i] = Complex(i < input.size() ? input[i] : 0.0f, 0.0f);
        }
        
        // Bit-reverse reordering
        for (size_t i = 1, j = 0; i < size; ++i) {
            size_t bit = size >> 1;
            for (; j & bit; bit >>= 1) {
                j ^= bit;
            }
            j ^= bit;
            
            if (i < j) {
                std::swap(output[i], output[j]);
            }
        }
        
        // FFT computation
        for (size_t len = 2; len <= size; len <<= 1) {
            float angle = -2.0f * M_PI / len;
            Complex wlen(cos(angle), sin(angle));
            
            for (size_t i = 0; i < size; i += len) {
                Complex w(1, 0);
                
                for (size_t j = 0; j < len / 2; ++j) {
                    Complex u = output[i + j];
                    Complex v = output[i + j + len / 2] * w;
                    
                    output[i + j] = u + v;
                    output[i + j + len / 2] = u - v;
                    
                    w *= wlen;
                }
            }
        }
    }
    
    /**
     * Convert complex FFT output to magnitude spectrum
     * @param fft_output Complex FFT output
     * @param magnitude Output magnitude spectrum
     */
    static void magnitude(const std::vector<Complex>& fft_output, std::vector<float>& magnitude) {
        magnitude.resize(fft_output.size() / 2); // Only use first half (positive frequencies)
        
        for (size_t i = 0; i < magnitude.size(); ++i) {
            magnitude[i] = std::abs(fft_output[i]);
        }
    }
    
    /**
     * Apply windowing function to reduce spectral leakage
     * @param input Input samples
     * @param windowed Output windowed samples
     */
    static void applyHannWindow(const std::vector<float>& input, std::vector<float>& windowed) {
        windowed.resize(input.size());
        
        for (size_t i = 0; i < input.size(); ++i) {
            float window = 0.5f * (1.0f - cos(2.0f * M_PI * i / (input.size() - 1)));
            windowed[i] = input[i] * window;
        }
    }
    
    /**
     * Get the next power of 2 greater than or equal to n
     */
    static size_t nextPowerOf2(size_t n) {
        if (n == 0) return 1;
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        return n + 1;
    }
};

} // namespace audio
} // namespace clap_jules