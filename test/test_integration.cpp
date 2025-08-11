#include <gtest/gtest.h>
#include "../src/plugin_model.h"
#include "../src/audio_processor.h"
#include "../src/plugin_viewmodel.h"
#include "../src/brisk_ui_view.h"
#include <memory>
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <vector>

// Integration test for the complete MVVM system
TEST(IntegrationTest, CompleteFilterSystem) {
    // Create dependency injection container
    auto injector = plugin::CreateDIContainer();
    
    // Create all components
    auto model = injector.create<std::shared_ptr<plugin::PluginModel>>();
    auto processor = injector.create<std::shared_ptr<plugin::AudioProcessor>>();
    auto viewmodel = injector.create<std::shared_ptr<plugin::PluginViewModel>>();
    auto ui_view = injector.create<std::shared_ptr<plugin::BriskUIView>>();
    
    // Initialize audio processor
    processor->Initialize(44100.0);
    
    // Set up test parameters
    double test_frequency = 2000.0;
    viewmodel->OnCutoffFrequencyChanged(test_frequency);
    
    // Verify parameter propagation
    EXPECT_EQ(model->GetCutoffFrequency(), test_frequency);
    EXPECT_EQ(viewmodel->GetCurrentCutoffFrequency(), test_frequency);
    
    // Test audio processing with frequency sweep
    const uint32_t frames = 256;
    std::vector<float> input(frames);
    std::vector<float> output(frames);
    
    // Create test signal: impulse + sine wave
    for (uint32_t i = 0; i < frames; ++i) {
        if (i == 0) {
            input[i] = 1.0f; // Impulse
        } else if (i < 64) {
            // High frequency content that should be filtered
            input[i] = 0.5f * sin(2.0f * M_PI * 8000.0f * i / 44100.0f);
        } else {
            input[i] = 0.0f;
        }
    }
    
    // Process audio
    processor->ProcessAudio(input.data(), output.data(), frames);
    
    // Verify filtering occurred
    bool filtering_applied = false;
    for (uint32_t i = 1; i < frames; ++i) {
        if (std::abs(output[i] - input[i]) > 1e-6f) {
            filtering_applied = true;
            break;
        }
    }
    EXPECT_TRUE(filtering_applied);
    
    // Test UI initialization
    void* dummy_handle = reinterpret_cast<void*>(0x12345678);
    EXPECT_TRUE(ui_view->Initialize(dummy_handle));
    
    // Test UI parameter updates
    ui_view->UpdateUI();
    // Note: In a real implementation, we would verify UI state
}

// Test filter frequency response characteristics
TEST(IntegrationTest, FilterFrequencyResponse) {
    auto model = std::make_shared<plugin::PluginModel>();
    plugin::AudioProcessor processor(model);
    
    processor.Initialize(44100.0);
    
    // Set cutoff to 1kHz
    processor.UpdateCutoffFrequency(1000.0);
    
    const uint32_t frames = 1024;
    std::vector<float> input(frames);
    std::vector<float> output(frames);
    
    // Test with different frequencies
    std::vector<double> test_frequencies = {100.0, 500.0, 1000.0, 2000.0, 5000.0};
    
    for (double freq : test_frequencies) {
        // Generate sine wave at test frequency
        for (uint32_t i = 0; i < frames; ++i) {
            input[i] = sin(2.0 * M_PI * freq * i / 44100.0);
        }
        
        // Process audio
        processor.ProcessAudio(input.data(), output.data(), frames);
        
        // Calculate RMS of input and output
        double input_rms = 0.0, output_rms = 0.0;
        for (uint32_t i = 0; i < frames; ++i) {
            input_rms += input[i] * input[i];
            output_rms += output[i] * output[i];
        }
        input_rms = sqrt(input_rms / frames);
        output_rms = sqrt(output_rms / frames);
        
        double attenuation_db = 20.0 * log10(output_rms / input_rms);
        
        // The simple one-pole filter has a -6dB/octave rolloff
        // Adjust expectations based on actual filter behavior
        if (freq <= 500.0) {
            EXPECT_GT(attenuation_db, -12.0); // Less than 12dB attenuation for low freq
        } else if (freq >= 2000.0) {
            EXPECT_LT(attenuation_db, -10.0); // More than 10dB attenuation for high freq
        }
        
        printf("Frequency: %.0f Hz, Attenuation: %.2f dB\n", freq, attenuation_db);
    }
}

// Test boost-di dependency injection
TEST(IntegrationTest, DependencyInjectionLifecycle) {
    auto injector = plugin::CreateDIContainer();
    
    // Create multiple instances - should get same singletons
    auto model1 = injector.create<std::shared_ptr<plugin::PluginModel>>();
    auto model2 = injector.create<std::shared_ptr<plugin::PluginModel>>();
    
    EXPECT_EQ(model1.get(), model2.get()); // Same instance due to singleton
    
    // Test parameter consistency across injected components
    auto viewmodel = injector.create<std::shared_ptr<plugin::PluginViewModel>>();
    auto processor = injector.create<std::shared_ptr<plugin::AudioProcessor>>();
    
    processor->Initialize(48000.0);
    viewmodel->OnCutoffFrequencyChanged(3000.0);
    
    EXPECT_EQ(model1->GetCutoffFrequency(), 3000.0);
    EXPECT_EQ(viewmodel->GetCurrentCutoffFrequency(), 3000.0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
