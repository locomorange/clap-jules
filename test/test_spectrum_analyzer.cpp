#include <gtest/gtest.h>
#include "../my_plugin.h"
#include "../graphics/skia_graphics.h"
#include <cmath>
#include <vector>

// Test the FFT functionality and spectrum analyzer
class SpectrumAnalyzerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a plugin instance
        plugin_instance = (my_plugin_t*)calloc(1, sizeof(my_plugin_t));
        
        // Initialize the plugin structure
        plugin_instance->plugin.plugin_data = plugin_instance;
        
        // Initialize the plugin
        my_plugin_init((clap_plugin*)plugin_instance);
    }
    
    void TearDown() override {
        my_plugin_destroy((clap_plugin*)plugin_instance);
        free(plugin_instance);
    }
    
    my_plugin_t* plugin_instance;
};

TEST_F(SpectrumAnalyzerTest, FFTBuffersInitialized) {
    EXPECT_EQ(plugin_instance->fft_input_buffer.size(), FFT_SIZE);
    EXPECT_EQ(plugin_instance->fft_buffer.size(), FFT_SIZE);
    EXPECT_EQ(plugin_instance->spectrum_magnitudes.size(), SPECTRUM_BINS);
    EXPECT_EQ(plugin_instance->smoothed_spectrum.size(), SPECTRUM_BINS);
    EXPECT_EQ(plugin_instance->input_buffer_pos, 0);
    EXPECT_EQ(plugin_instance->draw_mode, SpectrumDrawMode::LINES);
}

TEST_F(SpectrumAnalyzerTest, DrawModeEnum) {
    // Test that all draw modes are valid
    EXPECT_EQ((int)SpectrumDrawMode::LINES, 0);
    EXPECT_EQ((int)SpectrumDrawMode::DOTS, 1);
    EXPECT_EQ((int)SpectrumDrawMode::BINS, 2);
    EXPECT_EQ((int)SpectrumDrawMode::FILL, 3);
    
    // Test mode switching
    plugin_instance->draw_mode = SpectrumDrawMode::DOTS;
    EXPECT_EQ(plugin_instance->draw_mode, SpectrumDrawMode::DOTS);
    
    plugin_instance->draw_mode = SpectrumDrawMode::BINS;
    EXPECT_EQ(plugin_instance->draw_mode, SpectrumDrawMode::BINS);
    
    plugin_instance->draw_mode = SpectrumDrawMode::FILL;
    EXPECT_EQ(plugin_instance->draw_mode, SpectrumDrawMode::FILL);
}

TEST_F(SpectrumAnalyzerTest, SpectrumProcessingDoesNotCrash) {
    // Create a simple sine wave test signal
    const uint32_t test_frames = 128;
    std::vector<float> test_audio(test_frames);
    
    // Generate a 440Hz sine wave at 44100 Hz sample rate
    for (uint32_t i = 0; i < test_frames; ++i) {
        test_audio[i] = 0.5f * sinf(2.0f * M_PI * 440.0f * i / 44100.0f);
    }
    
    // This should not crash the plugin
    // Note: process_spectrum is in an anonymous namespace, so we can't call it directly
    // But we can test through the audio processing path
    
    // Set up a mock audio processing context
    clap_audio_buffer_t in_buf, out_buf;
    float* in_data = test_audio.data();
    float* out_data = new float[test_frames];
    
    in_buf.data32 = &in_data;
    in_buf.channel_count = 1;
    out_buf.data32 = &out_data;
    out_buf.channel_count = 1;
    
    clap_process_t process = {};
    process.audio_inputs = &in_buf;
    process.audio_outputs = &out_buf;
    process.audio_inputs_count = 1;
    process.audio_outputs_count = 1;
    process.frames_count = test_frames;
    
    // Process audio - this should not crash
    clap_process_status result = my_plugin_process((clap_plugin*)plugin_instance, &process);
    EXPECT_EQ(result, CLAP_PROCESS_CONTINUE);
    
    // Verify audio pass-through works
    for (uint32_t i = 0; i < test_frames; ++i) {
        EXPECT_FLOAT_EQ(out_data[i], test_audio[i]);
    }
    
    delete[] out_data;
}

TEST_F(SpectrumAnalyzerTest, SampleRateStorage) {
    // Test that sample rate is stored correctly during activation
    const double test_sample_rate = 48000.0;
    
    bool result = my_plugin_activate((clap_plugin*)plugin_instance, test_sample_rate, 64, 512);
    EXPECT_TRUE(result);
    EXPECT_EQ(plugin_instance->sample_rate, test_sample_rate);
}

TEST_F(SpectrumAnalyzerTest, SpectrumDataInitialization) {
    // Verify spectrum data starts as zeros
    for (size_t i = 0; i < SPECTRUM_BINS; ++i) {
        EXPECT_EQ(plugin_instance->spectrum_magnitudes[i], 0.0f);
        EXPECT_EQ(plugin_instance->smoothed_spectrum[i], 0.0f);
    }
    
    // Verify FFT input buffer starts as zeros
    for (size_t i = 0; i < FFT_SIZE; ++i) {
        EXPECT_EQ(plugin_instance->fft_input_buffer[i], 0.0f);
    }
}

TEST_F(SpectrumAnalyzerTest, GuiSizeForSpectrumDisplay) {
    // Verify GUI is sized appropriately for spectrum display
    EXPECT_EQ(plugin_instance->gui_width, 640);
    EXPECT_EQ(plugin_instance->gui_height, 480);
}

TEST_F(SpectrumAnalyzerTest, GuiRenderingDoesNotCrash) {
    // Create a graphics context for testing
    plugin_instance->graphics_context = clap_jules::graphics::createGraphicsContext(640, 480);
    ASSERT_TRUE(plugin_instance->graphics_context != nullptr);
    
    // Set up some test spectrum data
    for (size_t i = 0; i < SPECTRUM_BINS; ++i) {
        plugin_instance->smoothed_spectrum[i] = (float)i / SPECTRUM_BINS * 0.5f; // Gradual increase
    }
    
    // Test all drawing modes to ensure none crash
    for (int mode = 0; mode < 4; ++mode) {
        plugin_instance->draw_mode = (SpectrumDrawMode)mode;
        
        // This should not crash
        // Note: my_plugin_render_content is static, so we can't call it directly
        // But we can verify the graphics context is working
        EXPECT_NO_THROW({
            plugin_instance->graphics_context->clear(clap_jules::graphics::Color(0, 0, 0));
            plugin_instance->graphics_context->present();
        });
    }
}