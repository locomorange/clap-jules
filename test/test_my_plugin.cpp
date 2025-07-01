#include <gtest/gtest.h>
#include "../src/AudioProcessor.h"
#include "../src/MVVM.h"
#include "../src/DIContainer.h"
#include "../src/ClapParameterExtension.h"

// Placeholder for functions from my_clap_plugin.h
// For example, if you have a function like:
// int add(int a, int b);
// You can include the header:
// #include "../my_plugin.h"

TEST(MyPluginTest, PlaceholderTest) {
    ASSERT_EQ(1, 1);
    // Replace with actual tests for your plugin
    // For example:
    // EXPECT_EQ(add(2, 2), 4);
}

// Test AudioProcessor functionality
TEST(AudioProcessorTest, BasicFunctionality) {
    ClapeJules::AudioProcessor processor;
    
    // Test frequency setting
    processor.setFrequency(500.0f);
    EXPECT_EQ(processor.getFrequency(), 500.0f);
    
    // Test sample rate setting
    processor.setSampleRate(48000.0);
    
    // Test that filter doesn't crash with null inputs
    processor.process(nullptr, nullptr, 0, 0);
}

// Test MVVM structure
TEST(MVVMTest, ParameterBinding) {
    auto audioProcessor = std::make_shared<ClapeJules::AudioProcessor>();
    auto viewModel = std::make_shared<ClapeJules::FilterViewModel>(audioProcessor);
    auto view = std::make_shared<ClapeJules::FilterView>(viewModel);
    
    auto freqParam = view->getFrequencyParameter();
    ASSERT_NE(freqParam, nullptr);
    
    // Test parameter value setting
    freqParam->setValue(1500.0f);
    EXPECT_EQ(freqParam->getValue(), 1500.0f);
    EXPECT_EQ(audioProcessor->getFrequency(), 1500.0f);
}

// Test dependency injection
TEST(DITest, ContainerCreation) {
    auto container = ClapeJules::DI::createContainer();
    
    // Test that we can create instances through DI
    auto audioProcessor = container.create<std::shared_ptr<ClapeJules::AudioProcessor>>();
    auto viewModel = container.create<std::shared_ptr<ClapeJules::FilterViewModel>>();
    auto view = container.create<std::shared_ptr<ClapeJules::FilterView>>();
    
    ASSERT_NE(audioProcessor, nullptr);
    ASSERT_NE(viewModel, nullptr);
    ASSERT_NE(view, nullptr);
}

// Test CLAP parameter extension
TEST(ClapParameterTest, ParameterInfo) {
    // Test parameter count
    EXPECT_EQ(ClapeJules::ClapExt::ParameterExtension::count(nullptr), 1);
    
    // Test parameter info
    clap_param_info param_info;
    EXPECT_TRUE(ClapeJules::ClapExt::ParameterExtension::get_info(nullptr, 0, &param_info));
    EXPECT_EQ(param_info.id, 1);
    EXPECT_STREQ(param_info.name, "Frequency");
    EXPECT_EQ(param_info.min_value, 20.0);
    EXPECT_EQ(param_info.max_value, 20000.0);
    EXPECT_EQ(param_info.default_value, 1000.0);
    
    // Test invalid parameter index
    EXPECT_FALSE(ClapeJules::ClapExt::ParameterExtension::get_info(nullptr, 1, &param_info));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
