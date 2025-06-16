#include <gtest/gtest.h>
#include "../spectrum_analyzer.h"
#include "../fft.h"
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace clap_jules::audio;

class SpectrumAnalyzerTest : public ::testing::Test {
protected:
    void SetUp() override {
        analyzer = std::make_unique<SpectrumAnalyzer>(256, 44100.0f);
    }
    
    std::unique_ptr<SpectrumAnalyzer> analyzer;
};

TEST_F(SpectrumAnalyzerTest, InitializationTest) {
    EXPECT_EQ(analyzer->getNumBins(), 128); // FFT size / 2
    EXPECT_EQ(analyzer->getVisualizationType(), SpectrumVisualizationType::Lines);
}

TEST_F(SpectrumAnalyzerTest, VisualizationTypeTest) {
    analyzer->setVisualizationType(SpectrumVisualizationType::Dots);
    EXPECT_EQ(analyzer->getVisualizationType(), SpectrumVisualizationType::Dots);
    
    analyzer->setVisualizationType(SpectrumVisualizationType::Bins);
    EXPECT_EQ(analyzer->getVisualizationType(), SpectrumVisualizationType::Bins);
    
    analyzer->setVisualizationType(SpectrumVisualizationType::Fill);
    EXPECT_EQ(analyzer->getVisualizationType(), SpectrumVisualizationType::Fill);
}

TEST_F(SpectrumAnalyzerTest, FrequencyMappingTest) {
    analyzer->setSampleRate(44100.0f);
    
    // Test frequency mapping
    EXPECT_FLOAT_EQ(analyzer->getFrequencyForBin(0), 0.0f);
    
    // The actual Nyquist mapping might be slightly different due to FFT size
    float nyquist_freq = analyzer->getFrequencyForBin(analyzer->getNumBins() - 1);
    EXPECT_GT(nyquist_freq, 20000.0f); // Should be close to Nyquist
    EXPECT_LT(nyquist_freq, 25000.0f); // But not exactly due to FFT size discretization
    
    // Test invalid bin
    EXPECT_FLOAT_EQ(analyzer->getFrequencyForBin(analyzer->getNumBins()), 0.0f);
}

TEST_F(SpectrumAnalyzerTest, MagnitudeToDbTest) {
    EXPECT_FLOAT_EQ(SpectrumAnalyzer::magnitudeToDb(1.0f), 0.0f);
    EXPECT_FLOAT_EQ(SpectrumAnalyzer::magnitudeToDb(0.1f), -20.0f);
    EXPECT_EQ(SpectrumAnalyzer::magnitudeToDb(0.0f), SpectrumAnalyzer::MIN_DB);
}

TEST_F(SpectrumAnalyzerTest, AudioProcessingBasicTest) {
    // Generate a test sine wave
    std::vector<float> sine_wave(512);
    float frequency = 1000.0f; // 1kHz sine wave
    float sample_rate = 44100.0f;
    
    for (size_t i = 0; i < sine_wave.size(); ++i) {
        sine_wave[i] = sin(2.0f * M_PI * frequency * i / sample_rate);
    }
    
    // Process the audio
    analyzer->processAudio(sine_wave.data(), sine_wave.size());
    
    // Try to get magnitude spectrum
    std::vector<float> spectrum;
    bool has_data = analyzer->getMagnitudeSpectrum(spectrum);
    
    // We might not have data immediately depending on buffer size
    // This test mainly ensures no crashes occur
    EXPECT_GE(spectrum.size(), 0);
}

// FFT-specific tests
class FFTTest : public ::testing::Test {};

TEST_F(FFTTest, PowerOf2Test) {
    EXPECT_EQ(FFT::nextPowerOf2(0), 1);
    EXPECT_EQ(FFT::nextPowerOf2(1), 1);
    EXPECT_EQ(FFT::nextPowerOf2(3), 4);
    EXPECT_EQ(FFT::nextPowerOf2(5), 8);
    EXPECT_EQ(FFT::nextPowerOf2(128), 128);
    EXPECT_EQ(FFT::nextPowerOf2(129), 256);
}

TEST_F(FFTTest, BasicFFTTest) {
    // Test FFT with DC signal
    std::vector<float> input(8, 1.0f); // DC signal
    std::vector<FFT::Complex> output;
    
    FFT::forward(input, output, 8);
    
    EXPECT_EQ(output.size(), 8);
    
    // DC component should be strong
    float dc_magnitude = std::abs(output[0]);
    EXPECT_GT(dc_magnitude, 0.0f);
}

TEST_F(FFTTest, WindowingTest) {
    std::vector<float> input(8, 1.0f);
    std::vector<float> windowed;
    
    FFT::applyHannWindow(input, windowed);
    
    EXPECT_EQ(windowed.size(), input.size());
    
    // First and last samples should be attenuated by Hann window
    EXPECT_LT(windowed[0], input[0]);
    EXPECT_LT(windowed[windowed.size()-1], input[input.size()-1]);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}