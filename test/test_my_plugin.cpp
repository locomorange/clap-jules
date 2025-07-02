#include <gtest/gtest.h>
#include "../src/plugin_model.h"
#include "../src/audio_processor.h"
#include "../src/plugin_viewmodel.h"
#include <memory>
#include <cmath>

// Test the plugin model
TEST(PluginModelTest, ParameterSettersAndGetters) {
    plugin::PluginModel model;
    
    // Test default values
    EXPECT_EQ(model.GetSampleRate(), 44100.0);
    EXPECT_EQ(model.GetCutoffFrequency(), 1000.0);
    
    // Test setters
    model.SetSampleRate(48000.0);
    model.SetCutoffFrequency(2000.0);
    
    EXPECT_EQ(model.GetSampleRate(), 48000.0);
    EXPECT_EQ(model.GetCutoffFrequency(), 2000.0);
}

// Test the audio processor
TEST(AudioProcessorTest, BasicFiltering) {
    auto model = std::make_shared<plugin::PluginModel>();
    plugin::AudioProcessor processor(model);
    
    // Initialize processor
    processor.Initialize(44100.0);
    
    // Create test audio data
    const uint32_t frames = 64;
    float input[frames];
    float output[frames];
    
    // Fill input with impulse
    for (uint32_t i = 0; i < frames; ++i) {
        input[i] = (i == 0) ? 1.0f : 0.0f;
    }
    
    // Process audio
    processor.ProcessAudio(input, output, frames);
    
    // Check that output is not identical to input (filter is working)
    bool different = false;
    for (uint32_t i = 0; i < frames; ++i) {
        if (std::abs(output[i] - input[i]) > 1e-6f) {
            different = true;
            break;
        }
    }
    EXPECT_TRUE(different);
    
    // First sample should be affected by filter
    EXPECT_NE(output[0], input[0]);
}

// Test the view model
TEST(PluginViewModelTest, ParameterUpdates) {
    auto model = std::make_shared<plugin::PluginModel>();
    auto processor = std::make_shared<plugin::AudioProcessor>(model);
    plugin::PluginViewModel viewmodel(model, processor);
    
    // Test initial values
    EXPECT_EQ(viewmodel.GetCurrentCutoffFrequency(), 1000.0);
    EXPECT_EQ(viewmodel.GetCurrentSampleRate(), 44100.0);
    
    // Test parameter updates
    viewmodel.OnCutoffFrequencyChanged(2500.0);
    EXPECT_EQ(viewmodel.GetCurrentCutoffFrequency(), 2500.0);
}

// Test dependency injection
TEST(DependencyInjectionTest, ContainerCreation) {
    auto injector = plugin::CreateDIContainer();
    
    // Test that we can create instances
    auto model = injector.create<std::shared_ptr<plugin::PluginModel>>();
    auto processor = injector.create<std::shared_ptr<plugin::AudioProcessor>>();
    auto viewmodel = injector.create<std::shared_ptr<plugin::PluginViewModel>>();
    
    EXPECT_NE(model, nullptr);
    EXPECT_NE(processor, nullptr);
    EXPECT_NE(viewmodel, nullptr);
    
    // Test that singleton works (same instance)
    auto model2 = injector.create<std::shared_ptr<plugin::PluginModel>>();
    EXPECT_EQ(model.get(), model2.get());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
