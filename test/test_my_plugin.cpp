#include <gtest/gtest.h>
#include "../my_plugin.h"
#include <vector>
#include <cmath>

// Test the FFT and spectrum analysis functionality
TEST(SpectrumAnalyzerTest, FFTInitialization) {
    // Test FFT processor creation and initialization
    FFTProcessor* processor = fft_processor_create(44100.0);
    EXPECT_NE(processor, nullptr);
    
    // Test getting spectrum data (should return false initially since no data processed)
    float magnitudes[SPECTRUM_FFT_SIZE / 2];
    float frequencies[SPECTRUM_FFT_SIZE / 2]; 
    size_t count = 0;
    
    fft_processor_get_spectrum_data(processor, magnitudes, frequencies, &count);
    EXPECT_EQ(count, SPECTRUM_FFT_SIZE / 2);
    
    fft_processor_destroy(processor);
}

TEST(SpectrumAnalyzerTest, WindowFunction) {
    // Test Hann window generation
    std::vector<float> window(1024);
    generate_hann_window(window, 1024);
    
    // Check that first and last samples are close to zero
    EXPECT_NEAR(window[0], 0.0f, 0.001f);
    EXPECT_NEAR(window[1023], 0.0f, 0.001f);
    
    // Check that middle sample is close to 1
    EXPECT_NEAR(window[512], 1.0f, 0.001f);
    
    // Check symmetry
    for (size_t i = 0; i < 512; ++i) {
        EXPECT_NEAR(window[i], window[1023 - i], 0.001f);
    }
}

TEST(SpectrumAnalyzerTest, PluginHelpers) {
    // Create a mock plugin instance
    my_plugin_t plugin;
    memset(&plugin, 0, sizeof(plugin));
    
    // Initialize spectrum analyzer and FFT processor
    plugin.spectrum_analyzer = spectrum_analyzer_create();
    plugin.fft_processor = fft_processor_create(44100.0);
    
    EXPECT_NE(plugin.spectrum_analyzer, nullptr);
    EXPECT_NE(plugin.fft_processor, nullptr);
    
    // Test spectrum enable/disable
    set_plugin_spectrum_enabled(&plugin, true);
    EXPECT_TRUE(plugin.params.spectrum_enabled);
    
    set_plugin_spectrum_enabled(&plugin, false);
    EXPECT_FALSE(plugin.params.spectrum_enabled);
    
    // Test spectrum style setting
    set_plugin_spectrum_style(&plugin, SPECTRUM_STYLE_DOTS);
    EXPECT_EQ(plugin.params.spectrum_style, SPECTRUM_STYLE_DOTS);
    
    // Test getting spectrum data (should return false when no data ready initially)
    std::vector<float> magnitudes, frequencies;
    EXPECT_FALSE(get_plugin_spectrum_data(&plugin, magnitudes, frequencies));
    
    // Simulate some spectrum data by setting up test data
    float test_magnitudes[SPECTRUM_FFT_SIZE / 2];
    float test_frequencies[SPECTRUM_FFT_SIZE / 2];
    
    for (size_t i = 0; i < SPECTRUM_FFT_SIZE / 2; ++i) {
        test_magnitudes[i] = (float)i / 100.0f;
        test_frequencies[i] = (float)i * 20.0f;
    }
    
    // Update spectrum analyzer with test data
    spectrum_analyzer_update_data(plugin.spectrum_analyzer, test_magnitudes, test_frequencies, SPECTRUM_FFT_SIZE / 2);
    spectrum_analyzer_set_enabled(plugin.spectrum_analyzer, true);  // Make sure it's enabled
    
    // Now should return true and copy data
    EXPECT_TRUE(get_plugin_spectrum_data(&plugin, magnitudes, frequencies));
    EXPECT_EQ(magnitudes.size(), SPECTRUM_FFT_SIZE / 2);
    EXPECT_EQ(frequencies.size(), SPECTRUM_FFT_SIZE / 2);
    
    // Cleanup
    spectrum_analyzer_destroy(plugin.spectrum_analyzer);
    fft_processor_destroy(plugin.fft_processor);
}

TEST(SpectrumAnalyzerTest, FrequencyRange) {
    // Test that frequency calculation is in audible range
    FFTProcessor* processor = fft_processor_create(44100.0);
    EXPECT_NE(processor, nullptr);
    
    float magnitudes[SPECTRUM_FFT_SIZE / 2];
    float frequencies[SPECTRUM_FFT_SIZE / 2];
    size_t count = 0;
    
    fft_processor_get_spectrum_data(processor, magnitudes, frequencies, &count);
    EXPECT_EQ(count, SPECTRUM_FFT_SIZE / 2);
    
    // Check that frequencies are in expected range (should cover 0 to 22050 Hz)
    EXPECT_GE(frequencies[0], 0.0f);
    EXPECT_LE(frequencies[count - 1], 22050.0f);
    
    // Check that frequencies are monotonically increasing
    for (size_t i = 1; i < count; ++i) {
        EXPECT_GT(frequencies[i], frequencies[i-1]);
    }
    
    fft_processor_destroy(processor);
}

TEST(MyPluginTest, PlaceholderTest) {
    ASSERT_EQ(1, 1);
    // Replace with actual tests for your plugin
    // For example:
    // EXPECT_EQ(add(2, 2), 4);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
