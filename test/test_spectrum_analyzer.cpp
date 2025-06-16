#include <gtest/gtest.h>
#include "../spectrum_analyzer.h"
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>

class SpectrumAnalyzerTest : public ::testing::Test {
protected:
    void SetUp() override {
        analyzer = std::make_unique<clap_jules::SpectrumAnalyzer>(512, 44100.0);
    }

    std::unique_ptr<clap_jules::SpectrumAnalyzer> analyzer;
};

TEST_F(SpectrumAnalyzerTest, InitializationTest) {
    EXPECT_EQ(analyzer->getBinCount(), 256); // FFT size / 2
    EXPECT_EQ(analyzer->getFFTSize(), 512);
    EXPECT_EQ(analyzer->getDrawMode(), clap_jules::SpectrumDrawMode::Lines);
}

TEST_F(SpectrumAnalyzerTest, DrawModeControl) {
    // Test all drawing modes
    analyzer->setDrawMode(clap_jules::SpectrumDrawMode::Dots);
    EXPECT_EQ(analyzer->getDrawMode(), clap_jules::SpectrumDrawMode::Dots);
    
    analyzer->setDrawMode(clap_jules::SpectrumDrawMode::Bins);
    EXPECT_EQ(analyzer->getDrawMode(), clap_jules::SpectrumDrawMode::Bins);
    
    analyzer->setDrawMode(clap_jules::SpectrumDrawMode::Fill);
    EXPECT_EQ(analyzer->getDrawMode(), clap_jules::SpectrumDrawMode::Fill);
    
    analyzer->setDrawMode(clap_jules::SpectrumDrawMode::Lines);
    EXPECT_EQ(analyzer->getDrawMode(), clap_jules::SpectrumDrawMode::Lines);
}

TEST_F(SpectrumAnalyzerTest, FrequencyCalculation) {
    // Test frequency calculation for known bins
    float freq_0 = analyzer->getFrequencyForBin(0);
    EXPECT_FLOAT_EQ(freq_0, 0.0f); // DC component
    
    float freq_max = analyzer->getFrequencyForBin(analyzer->getBinCount() - 1); 
    EXPECT_NEAR(freq_max, 22050.0f, 100.0f); // Near Nyquist frequency (44100/2)
    
    float freq_quarter = analyzer->getFrequencyForBin(analyzer->getBinCount() / 4);
    EXPECT_NEAR(freq_quarter, 5512.5f, 100.0f); // Quarter of Nyquist
}

TEST_F(SpectrumAnalyzerTest, AudioProcessing) {
    // Create test audio data - sine wave at known frequency
    std::vector<float> test_audio(256);
    float frequency = 1000.0f; // 1kHz sine wave
    float sample_rate = 44100.0f;
    
    for (size_t i = 0; i < test_audio.size(); ++i) {
        test_audio[i] = 0.5f * sin(2.0f * M_PI * frequency * i / sample_rate);
    }
    
    // Process the audio
    analyzer->processAudio(test_audio.data(), test_audio.size());
    
    // Get spectrum
    const auto& spectrum = analyzer->getSpectrum();
    EXPECT_EQ(spectrum.size(), analyzer->getBinCount());
    
    // All spectrum values should be non-negative and finite
    for (float value : spectrum) {
        EXPECT_GE(value, 0.0f);
        EXPECT_TRUE(std::isfinite(value));
    }
}

TEST_F(SpectrumAnalyzerTest, SpectrumDataSize) {
    // Initially spectrum should be empty or have the correct size
    const auto& spectrum = analyzer->getSpectrum();
    EXPECT_EQ(spectrum.size(), analyzer->getBinCount());
    
    // Process some audio data
    std::vector<float> test_audio(128, 0.1f); // Simple constant level
    analyzer->processAudio(test_audio.data(), test_audio.size());
    
    // Spectrum should still have correct size
    const auto& updated_spectrum = analyzer->getSpectrum();
    EXPECT_EQ(updated_spectrum.size(), analyzer->getBinCount());
}

class SimpleFFTTest : public ::testing::Test {
protected:
    void SetUp() override {
        fft = std::make_unique<clap_jules::SimpleFFT>(256);
    }

    std::unique_ptr<clap_jules::SimpleFFT> fft;
};

TEST_F(SimpleFFTTest, Initialization) {
    EXPECT_EQ(fft->getSize(), 256);
    EXPECT_EQ(fft->getBinCount(), 128); // Size / 2
}

TEST_F(SimpleFFTTest, SpectrumComputation) {
    // Create test input data
    std::vector<float> input(fft->getSize(), 0.0f);
    std::vector<float> spectrum;
    
    // Add a simple DC component
    for (size_t i = 0; i < input.size(); ++i) {
        input[i] = 1.0f; // DC signal
    }
    
    fft->computeSpectrum(input.data(), spectrum);
    
    // Check output size
    EXPECT_EQ(spectrum.size(), fft->getBinCount());
    
    // DC component should be largest (at bin 0)
    EXPECT_GT(spectrum[0], 0.0f);
    
    // All values should be non-negative and finite
    for (float value : spectrum) {
        EXPECT_GE(value, 0.0f);
        EXPECT_TRUE(std::isfinite(value));
    }
}

TEST_F(SimpleFFTTest, PowerOfTwoSize) {
    // Test that FFT handles non-power-of-2 sizes by rounding up
    auto fft_odd = std::make_unique<clap_jules::SimpleFFT>(300);
    EXPECT_EQ(fft_odd->getSize(), 512); // Next power of 2 after 300
    
    auto fft_power = std::make_unique<clap_jules::SimpleFFT>(1024);
    EXPECT_EQ(fft_power->getSize(), 1024); // Already power of 2
}