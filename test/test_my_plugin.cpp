#include <gtest/gtest.h>
#include "../graphics_wrapper.h"

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

TEST(GraphicsTest, GraphicsContextCreation) {
    // Test that graphics context can be created
    clap_jules::GraphicsContext graphics(100, 100);
    EXPECT_EQ(graphics.getWidth(), 100);
    EXPECT_EQ(graphics.getHeight(), 100);
    EXPECT_NE(graphics.getPixels(), nullptr);
}

TEST(GraphicsTest, BasicDrawingOperations) {
    // Test basic drawing operations
    clap_jules::GraphicsContext graphics(50, 50);
    
    // Clear with a specific color
    graphics.clear(0xFF123456);
    
    // Draw some shapes
    graphics.drawRect(10, 10, 20, 20, 0xFFFF0000);
    graphics.drawCircle(25, 25, 10, 0xFF00FF00);
    graphics.drawLine(0, 0, 49, 49, 0xFF0000FF);
    
    // Verify pixels are accessible
    const uint32_t* pixels = graphics.getPixels();
    EXPECT_NE(pixels, nullptr);
}

TEST(GraphicsTest, SkiaAvailabilityCheck) {
    // Test Skia availability reporting
    bool skiaAvailable = clap_jules::GraphicsContext::isSkiaAvailable();
    
    #ifdef HAVE_SKIA
    EXPECT_TRUE(skiaAvailable);
    #else
    EXPECT_FALSE(skiaAvailable);
    #endif
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
