#include <gtest/gtest.h>
#include "../filter.h"
#include "../mvvm_impl.h"
#include "../gui.h"

TEST(FilterTest, BasicFunctionality) {
    LowPassFilter filter;
    filter.setSampleRate(44100.0);
    filter.setFrequency(1000.0);
    
    // Test with some simple input
    float input[4] = {1.0f, 0.0f, -1.0f, 0.0f};
    float output[4];
    
    filter.process(input, output, 4);
    
    // Output should be different from input (filtered)
    EXPECT_NE(output[0], input[0]);
    EXPECT_NE(output[2], input[2]);
}

TEST(MVVMTest, ViewModelFunctionality) {
    auto model = std::make_shared<FilterAudioModel>();
    FilterPluginViewModel viewModel(model);
    
    // Test frequency setting
    viewModel.setFrequency(2000.0);
    EXPECT_EQ(viewModel.getFrequency(), 2000.0);
    
    // Test sample rate setting
    viewModel.setSampleRate(48000.0);
    
    // Test audio processing
    float input[8] = {1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f};
    float output[8];
    
    viewModel.processAudio(input, output, 4, 2); // 4 samples, 2 channels
    
    // Should process without crashing
    EXPECT_TRUE(true);
}

TEST(GUITest, SimpleGUIFunctionality) {
    SimplePluginGUI gui;
    
    // Test frequency setting
    gui.setFrequency(500.0);
    EXPECT_EQ(gui.getFrequency(), 500.0);
    
    // Test visibility
    EXPECT_FALSE(gui.isVisible());
    gui.show();
    EXPECT_TRUE(gui.isVisible());
    gui.hide();
    EXPECT_FALSE(gui.isVisible());
    
    // Test callback functionality
    double callbackFrequency = 0.0;
    gui.setFrequencyChangeCallback([&callbackFrequency](double freq) {
        callbackFrequency = freq;
    });
    
    gui.simulateFrequencyChange(1500.0);
    EXPECT_EQ(callbackFrequency, 1500.0);
    EXPECT_EQ(gui.getFrequency(), 1500.0);
}

TEST(MyPluginTest, PlaceholderTest) {
    ASSERT_EQ(1, 1);
    // Basic sanity test
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
