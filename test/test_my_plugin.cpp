#include <gtest/gtest.h>
#include "../my_plugin.h"
#include <vector>
#include <cmath>

// Test the FFT and spectrum analysis functionality
TEST(SpectrumAnalyzerTest, FFTInitialization) {
    // Test FFT data initialization
    fft_data_t fft_data;
    init_fft_data(&fft_data, 44100.0);
    
    // Check that buffers are properly sized
    EXPECT_EQ(fft_data.fft_buffer.size(), SPECTRUM_FFT_SIZE);
    EXPECT_EQ(fft_data.input_buffer.size(), SPECTRUM_FFT_SIZE);
    EXPECT_EQ(fft_data.window_function.size(), SPECTRUM_FFT_SIZE);
    EXPECT_EQ(fft_data.spectrum_data.magnitudes.size(), SPECTRUM_FFT_SIZE / 2);
    EXPECT_EQ(fft_data.spectrum_data.frequencies.size(), SPECTRUM_FFT_SIZE / 2);
    
    // Check sample rate is set correctly
    EXPECT_DOUBLE_EQ(fft_data.sample_rate, 44100.0);
    
    // Check initial state
    EXPECT_FALSE(fft_data.spectrum_data.data_ready.load());
    EXPECT_TRUE(fft_data.spectrum_data.enabled);
    EXPECT_EQ(fft_data.spectrum_data.draw_style, SPECTRUM_STYLE_LINES);
    
    cleanup_fft_data(&fft_data);
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
    
    // Initialize FFT data
    init_fft_data(&plugin.fft_data, 44100.0);
    
    // Test spectrum enable/disable
    set_plugin_spectrum_enabled(&plugin, true);
    EXPECT_TRUE(plugin.params.spectrum_enabled);
    EXPECT_TRUE(plugin.fft_data.spectrum_data.enabled);
    
    set_plugin_spectrum_enabled(&plugin, false);
    EXPECT_FALSE(plugin.params.spectrum_enabled);
    EXPECT_FALSE(plugin.fft_data.spectrum_data.enabled);
    
    // Test spectrum style setting
    set_plugin_spectrum_style(&plugin, SPECTRUM_STYLE_DOTS);
    EXPECT_EQ(plugin.params.spectrum_style, SPECTRUM_STYLE_DOTS);
    EXPECT_EQ(plugin.fft_data.spectrum_data.draw_style, SPECTRUM_STYLE_DOTS);
    
    // Test getting spectrum data (should return false when no data ready)
    std::vector<float> magnitudes, frequencies;
    EXPECT_FALSE(get_plugin_spectrum_data(&plugin, magnitudes, frequencies));
    
    // Simulate some data being ready
    plugin.fft_data.spectrum_data.data_ready.store(true);
    
    // Fill with test data
    for (size_t i = 0; i < plugin.fft_data.spectrum_data.magnitudes.size(); ++i) {
        plugin.fft_data.spectrum_data.magnitudes[i] = (float)i / 100.0f;
        plugin.fft_data.spectrum_data.frequencies[i] = (float)i * 20.0f;
    }
    
    // Now should return true and copy data
    EXPECT_TRUE(get_plugin_spectrum_data(&plugin, magnitudes, frequencies));
    EXPECT_EQ(magnitudes.size(), plugin.fft_data.spectrum_data.magnitudes.size());
    EXPECT_EQ(frequencies.size(), plugin.fft_data.spectrum_data.frequencies.size());
    
    cleanup_fft_data(&plugin.fft_data);
}

TEST(SpectrumAnalyzerTest, FrequencyRange) {
    // Test that frequency calculation is in audible range
    fft_data_t fft_data;
    init_fft_data(&fft_data, 44100.0);
    
    // Check that frequencies are in expected range (should cover 0 to 22050 Hz)
    EXPECT_GE(fft_data.spectrum_data.frequencies[0], 0.0f);
    EXPECT_LE(fft_data.spectrum_data.frequencies.back(), 22050.0f);
    
    // Check that frequencies are monotonically increasing
    for (size_t i = 1; i < fft_data.spectrum_data.frequencies.size(); ++i) {
        EXPECT_GT(fft_data.spectrum_data.frequencies[i], fft_data.spectrum_data.frequencies[i-1]);
    }
    
    cleanup_fft_data(&fft_data);
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
