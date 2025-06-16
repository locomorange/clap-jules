#include <iostream>
#include <memory>
#include <cmath>
#include <vector>
#include <cstdlib>
#include "my_plugin.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main() {
    std::cout << "=== CLAP-Jules VU Meter Demo ===" << std::endl;
    
    // Create plugin instance
    std::unique_ptr<my_plugin_t> plugin(
        static_cast<my_plugin_t*>(calloc(1, sizeof(my_plugin_t)))
    );
    
    if (!plugin) {
        std::cerr << "Failed to allocate plugin memory" << std::endl;
        return 1;
    }
    
    // Initialize plugin structure
    plugin->plugin.plugin_data = plugin.get();
    
    // Initialize VU meter state
    plugin->current_level_left = 0.0f;
    plugin->current_level_right = 0.0f;
    plugin->peak_level_left = 0.0f;
    plugin->peak_level_right = 0.0f;
    plugin->peak_hold_counter_left = 0;
    plugin->peak_hold_counter_right = 0;
    
    std::cout << "Plugin initialized successfully" << std::endl;
    
    // Simulate audio processing with different signal levels
    const uint32_t frames_count = 64;
    const uint32_t channels = 2;
    
    // Create audio buffers
    std::vector<float> input_left(frames_count);
    std::vector<float> input_right(frames_count);
    std::vector<float> output_left(frames_count);
    std::vector<float> output_right(frames_count);
    
    std::vector<float*> input_data = {input_left.data(), input_right.data()};
    std::vector<float*> output_data = {output_left.data(), output_right.data()};
    
    // Create CLAP audio buffers
    clap_audio_buffer_t input_buffer = {};
    input_buffer.data32 = input_data.data();
    input_buffer.channel_count = channels;
    
    clap_audio_buffer_t output_buffer = {};
    output_buffer.data32 = output_data.data();
    output_buffer.channel_count = channels;
    
    clap_process_t process = {};
    process.frames_count = frames_count;
    process.audio_inputs = &input_buffer;
    process.audio_inputs_count = 1;
    process.audio_outputs = &output_buffer;
    process.audio_outputs_count = 1;
    
    // Helper function to convert levels to dB
    auto levelToDb = [](float level) -> float {
        if (level <= 0.0f) return -60.0f;
        return 20.0f * log10f(level);
    };
    
    // Test different signal levels
    std::cout << "\\nTesting different signal levels:" << std::endl;
    std::cout << "Level\\t\\tLeft dB\\t\\tRight dB\\tPeak L\\t\\tPeak R" << std::endl;
    std::cout << "-----\\t\\t-------\\t\\t--------\\t------\\t\\t------" << std::endl;
    
    std::vector<float> test_levels = {0.0f, 0.1f, 0.3f, 0.5f, 0.7f, 0.9f, 1.0f};
    
    for (float level : test_levels) {
        // Generate test signal
        for (uint32_t i = 0; i < frames_count; ++i) {
            float sample = level * sinf(2.0f * M_PI * i / frames_count);
            input_left[i] = sample;
            input_right[i] = sample * 0.8f; // Right channel slightly lower
        }
        
        // Process audio (simulate the plugin's process function logic)
        float sum_left = 0.0f;
        float sum_right = 0.0f;
        float peak_left = 0.0f;
        float peak_right = 0.0f;
        
        for (uint32_t i = 0; i < frames_count; ++i) {
            float sample_left = input_left[i];
            float sample_right = input_right[i];
            
            // Pass-through audio
            output_left[i] = sample_left;
            output_right[i] = sample_right;
            
            // Calculate RMS and peak
            sum_left += sample_left * sample_left;
            sum_right += sample_right * sample_right;
            
            float abs_left = fabs(sample_left);
            float abs_right = fabs(sample_right);
            if (abs_left > peak_left) peak_left = abs_left;
            if (abs_right > peak_right) peak_right = abs_right;
        }
        
        // Calculate RMS levels
        float rms_left = sqrtf(sum_left / frames_count);
        float rms_right = sqrtf(sum_right / frames_count);
        
        // Smooth the levels (simulate plugin smoothing)
        const float smoothing = 0.1f;
        plugin->current_level_left = plugin->current_level_left * (1.0f - smoothing) + rms_left * smoothing;
        plugin->current_level_right = plugin->current_level_right * (1.0f - smoothing) + rms_right * smoothing;
        
        // Update peak levels
        if (peak_left > plugin->peak_level_left) {
            plugin->peak_level_left = peak_left;
        }
        if (peak_right > plugin->peak_level_right) {
            plugin->peak_level_right = peak_right;
        }
        
        // Display results
        float left_db = levelToDb(plugin->current_level_left);
        float right_db = levelToDb(plugin->current_level_right);
        float peak_left_db = levelToDb(plugin->peak_level_left);
        float peak_right_db = levelToDb(plugin->peak_level_right);
        
        printf("%.1f%%\\t\\t%.1fdB\\t\\t%.1fdB\\t\\t%.1fdB\\t\\t%.1fdB\\n", 
               level * 100.0f, left_db, right_db, peak_left_db, peak_right_db);
    }
    
    std::cout << "\\n=== VU Meter Demonstration Complete ===" << std::endl;
    std::cout << "\\nVU Meter Features:" << std::endl;
    std::cout << "- Real-time audio level monitoring" << std::endl;
    std::cout << "- Separate left and right channel meters" << std::endl;
    std::cout << "- RMS level calculation for true VU behavior" << std::endl;
    std::cout << "- Peak level detection with hold" << std::endl;
    std::cout << "- Color-coded display (red/yellow/green)" << std::endl;
    std::cout << "- dB scale from -60dB to 0dB" << std::endl;
    std::cout << "- Horizontal bar visualization" << std::endl;
    std::cout << "- Audio pass-through (no signal processing)" << std::endl;
    
    std::cout << "\\nUsage in DAW:" << std::endl;
    std::cout << "1. Load 'MyFirstClapPlugin.so' as a CLAP plugin" << std::endl;
    std::cout << "2. Insert on any audio track or bus" << std::endl;
    std::cout << "3. Open the plugin GUI to see the VU meter" << std::endl;
    std::cout << "4. Play audio to see real-time level monitoring" << std::endl;
    
    return 0;
}