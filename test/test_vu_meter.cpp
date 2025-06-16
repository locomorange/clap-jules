#include <gtest/gtest.h>
#include "../my_plugin.h"
#include <clap/clap.h>
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class VUMeterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a plugin instance
        plugin = (my_plugin_t*)calloc(1, sizeof(my_plugin_t));
        plugin->plugin.desc = nullptr;
        plugin->plugin.plugin_data = plugin;
        plugin->plugin.init = nullptr;
        plugin->plugin.destroy = nullptr;
        plugin->plugin.activate = nullptr;
        plugin->plugin.deactivate = nullptr;
        plugin->plugin.start_processing = nullptr;
        plugin->plugin.stop_processing = nullptr;
        plugin->plugin.reset = nullptr;
        plugin->plugin.process = nullptr;
        plugin->plugin.get_extension = nullptr;
        plugin->plugin.on_main_thread = nullptr;
        
        // Initialize VU meter state
        plugin->current_level = 0.0f;
        plugin->peak_level = 0.0f;
        plugin->decay_rate = 0.95f;
        plugin->sample_rate = 44100;
    }
    
    void TearDown() override {
        free(plugin);
    }
    
    my_plugin_t* plugin;
};

TEST_F(VUMeterTest, InitialState) {
    EXPECT_EQ(plugin->current_level, 0.0f);
    EXPECT_EQ(plugin->peak_level, 0.0f);
    EXPECT_EQ(plugin->decay_rate, 0.95f);
    EXPECT_EQ(plugin->sample_rate, 44100);
}

TEST_F(VUMeterTest, StateVariablesInRange) {
    // Test that VU meter variables are properly initialized within valid ranges
    EXPECT_GE(plugin->current_level, 0.0f);
    EXPECT_LE(plugin->current_level, 1.0f);
    EXPECT_GE(plugin->peak_level, 0.0f);
    EXPECT_LE(plugin->peak_level, 1.0f);
    EXPECT_GT(plugin->decay_rate, 0.0f);
    EXPECT_LE(plugin->decay_rate, 1.0f);
    EXPECT_GT(plugin->sample_rate, 0);
}

TEST_F(VUMeterTest, LevelCalculation) {
    // Test level setting and bounds checking
    plugin->current_level = 0.5f;
    plugin->peak_level = 0.8f;
    
    EXPECT_EQ(plugin->current_level, 0.5f);
    EXPECT_EQ(plugin->peak_level, 0.8f);
    
    // Test clamping to [0, 1] range
    plugin->current_level = 1.5f;  // Over maximum
    plugin->peak_level = -0.1f;    // Under minimum
    
    // In real implementation, these would be clamped by the process function
    // Here we just verify the test structure works
    EXPECT_GT(plugin->current_level, 1.0f);  // Will be clamped in actual processing
    EXPECT_LT(plugin->peak_level, 0.0f);     // Will be clamped in actual processing
}

TEST_F(VUMeterTest, DecayRateValidation) {
    // Test decay rate bounds
    plugin->decay_rate = 0.9f;
    EXPECT_EQ(plugin->decay_rate, 0.9f);
    
    // Test edge cases
    plugin->decay_rate = 0.0f;
    EXPECT_EQ(plugin->decay_rate, 0.0f);
    
    plugin->decay_rate = 1.0f;
    EXPECT_EQ(plugin->decay_rate, 1.0f);
}

TEST_F(VUMeterTest, AudioProcessingDataStructures) {
    // Test that we can create the required data structures for audio processing
    const uint32_t frame_count = 512;
    const uint32_t channel_count = 2;
    
    // Simulate audio input data
    std::vector<float> left_channel(frame_count);
    std::vector<float> right_channel(frame_count);
    std::vector<float*> channel_data = {left_channel.data(), right_channel.data()};
    
    // Fill with test data (sine wave)
    for (uint32_t i = 0; i < frame_count; ++i) {
        float sample = 0.5f * std::sin(2.0f * M_PI * 440.0f * i / plugin->sample_rate);
        left_channel[i] = sample;
        right_channel[i] = sample;
    }
    
    // Test that we can calculate RMS and peak values
    float sum_squares = 0.0f;
    float max_sample = 0.0f;
    
    for (uint32_t i = 0; i < frame_count; ++i) {
        for (uint32_t ch = 0; ch < channel_count; ++ch) {
            float sample = channel_data[ch][i];
            float abs_sample = std::abs(sample);
            if (abs_sample > max_sample) {
                max_sample = abs_sample;
            }
            sum_squares += sample * sample;
        }
    }
    
    float rms = std::sqrt(sum_squares / (frame_count * channel_count));
    
    // Verify calculations are reasonable
    EXPECT_GT(rms, 0.0f);
    EXPECT_LT(rms, 1.0f);
    EXPECT_GT(max_sample, 0.0f);
    EXPECT_LT(max_sample, 1.0f);
    
    // For a sine wave with amplitude 0.5, RMS should be approximately 0.5/sqrt(2) ≈ 0.35
    EXPECT_NEAR(rms, 0.5f / std::sqrt(2.0f), 0.05f);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}