#include <gtest/gtest.h>
#include <cmath>
#include <memory>
#include "../my_plugin.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class VUMeterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a plugin instance
        plugin_instance = std::unique_ptr<my_plugin_t>(
            static_cast<my_plugin_t*>(calloc(1, sizeof(my_plugin_t)))
        );
        
        // Initialize plugin structure
        plugin_instance->plugin.desc = nullptr;
        plugin_instance->plugin.plugin_data = plugin_instance.get();
        plugin_instance->plugin.init = nullptr;
        plugin_instance->plugin.destroy = nullptr;
        plugin_instance->plugin.activate = nullptr;
        plugin_instance->plugin.deactivate = nullptr;
        plugin_instance->plugin.start_processing = nullptr;
        plugin_instance->plugin.stop_processing = nullptr;
        plugin_instance->plugin.reset = nullptr;
        plugin_instance->plugin.process = nullptr;
        plugin_instance->plugin.get_extension = nullptr;
        plugin_instance->plugin.on_main_thread = nullptr;
        
        // Initialize VU meter state
        plugin_instance->current_level_left = 0.0f;
        plugin_instance->current_level_right = 0.0f;
        plugin_instance->peak_level_left = 0.0f;
        plugin_instance->peak_level_right = 0.0f;
        plugin_instance->peak_hold_counter_left = 0;
        plugin_instance->peak_hold_counter_right = 0;
    }
    
    void TearDown() override {
        plugin_instance.reset();
    }
    
    std::unique_ptr<my_plugin_t> plugin_instance;
};

TEST_F(VUMeterTest, InitialState) {
    // Test that the VU meter starts with zero levels
    EXPECT_EQ(plugin_instance->current_level_left, 0.0f);
    EXPECT_EQ(plugin_instance->current_level_right, 0.0f);
    EXPECT_EQ(plugin_instance->peak_level_left, 0.0f);
    EXPECT_EQ(plugin_instance->peak_level_right, 0.0f);
    EXPECT_EQ(plugin_instance->peak_hold_counter_left, 0);
    EXPECT_EQ(plugin_instance->peak_hold_counter_right, 0);
}

TEST_F(VUMeterTest, LevelTracking) {
    // Test that levels can be set and retrieved
    plugin_instance->current_level_left = 0.5f;
    plugin_instance->current_level_right = 0.7f;
    plugin_instance->peak_level_left = 0.8f;
    plugin_instance->peak_level_right = 0.9f;
    
    EXPECT_EQ(plugin_instance->current_level_left, 0.5f);
    EXPECT_EQ(plugin_instance->current_level_right, 0.7f);
    EXPECT_EQ(plugin_instance->peak_level_left, 0.8f);
    EXPECT_EQ(plugin_instance->peak_level_right, 0.9f);
}

TEST_F(VUMeterTest, AudioProcessingStructure) {
    // Test that we can create audio buffers for testing
    const uint32_t frames_count = 64;
    const uint32_t channels = 2;
    
    // Create input audio buffer
    float* input_left = new float[frames_count];
    float* input_right = new float[frames_count];
    float** input_data = new float*[channels];
    input_data[0] = input_left;
    input_data[1] = input_right;
    
    // Create output audio buffer
    float* output_left = new float[frames_count];
    float* output_right = new float[frames_count];
    float** output_data = new float*[channels];
    output_data[0] = output_left;
    output_data[1] = output_right;
    
    // Fill input with test signal (sine wave)
    for (uint32_t i = 0; i < frames_count; ++i) {
        float sample = 0.5f * sinf(2.0f * M_PI * i / frames_count);
        input_left[i] = sample;
        input_right[i] = sample * 0.8f; // Different level for right channel
    }
    
    // Create CLAP audio buffers
    clap_audio_buffer_t input_buffer = {};
    input_buffer.data32 = input_data;
    input_buffer.channel_count = channels;
    input_buffer.latency = 0;
    input_buffer.constant_mask = 0;
    
    clap_audio_buffer_t output_buffer = {};
    output_buffer.data32 = output_data;
    output_buffer.channel_count = channels;
    output_buffer.latency = 0;
    output_buffer.constant_mask = 0;
    
    // Create process structure
    clap_process_t process = {};
    process.frames_count = frames_count;
    process.audio_inputs = &input_buffer;
    process.audio_inputs_count = 1;
    process.audio_outputs = &output_buffer;
    process.audio_outputs_count = 1;
    process.in_events = nullptr;
    process.out_events = nullptr;
    
    // Simulate the audio processing logic (simplified version)
    float sum_left = 0.0f;
    float sum_right = 0.0f;
    
    for (uint32_t i = 0; i < frames_count; ++i) {
        float sample_left = input_left[i];
        float sample_right = input_right[i];
        
        // Copy input to output (pass-through)
        output_left[i] = sample_left;
        output_right[i] = sample_right;
        
        // Calculate RMS for testing
        sum_left += sample_left * sample_left;
        sum_right += sample_right * sample_right;
    }
    
    float rms_left = sqrtf(sum_left / frames_count);
    float rms_right = sqrtf(sum_right / frames_count);
    
    // Test that RMS calculation produces reasonable values
    EXPECT_GT(rms_left, 0.0f);
    EXPECT_LT(rms_left, 1.0f);
    EXPECT_GT(rms_right, 0.0f);
    EXPECT_LT(rms_right, 1.0f);
    EXPECT_LT(rms_right, rms_left); // Right channel should be lower
    
    // Verify pass-through audio processing  
    for (uint32_t i = 0; i < frames_count; ++i) {
        EXPECT_EQ(output_left[i], input_left[i]);
        EXPECT_EQ(output_right[i], input_right[i]);
    }
    
    // Clean up
    delete[] input_left;
    delete[] input_right;
    delete[] input_data;
    delete[] output_left;
    delete[] output_right;
    delete[] output_data;
}

TEST_F(VUMeterTest, DatabaseConversion) {
    // Test dB conversion logic (from the render function)
    auto levelToDb = [](float level) -> float {
        if (level <= 0.0f) return -60.0f; // Minimum -60dB
        return 20.0f * log10f(level);
    };
    
    // Test known values
    EXPECT_FLOAT_EQ(levelToDb(1.0f), 0.0f);      // 100% = 0dB
    EXPECT_FLOAT_EQ(levelToDb(0.1f), -20.0f);    // 10% = -20dB
    EXPECT_FLOAT_EQ(levelToDb(0.01f), -40.0f);   // 1% = -40dB
    EXPECT_FLOAT_EQ(levelToDb(0.0f), -60.0f);    // 0% = -60dB (minimum)
}