#include <gtest/gtest.h>
#include "../my_plugin.h"

TEST(MyPluginTest, PlaceholderTest) {
    ASSERT_EQ(1, 1);
}

TEST(MyPluginTest, GraphicsLibrariesIntegration) {
    // Test that graphics libraries are properly integrated at compile time
#ifdef HAVE_GLFW
    EXPECT_TRUE(true) << "GLFW support is enabled";
#else
    FAIL() << "GLFW support should be enabled in this build";
#endif

#ifdef HAVE_SKIA
    EXPECT_TRUE(true) << "Skia support is enabled";
    // Note: Skia is optional and may not be enabled in all builds
#endif
}

TEST(MyPluginTest, PluginStructureWithGraphics) {
    // Test that the plugin structure includes graphics fields when available
    my_plugin_t plugin_instance;
    
    // Initialize the basic plugin structure
    plugin_instance.plugin.desc = nullptr;
    plugin_instance.plugin.plugin_data = &plugin_instance;
    
#ifdef HAVE_GLFW
    // Test GLFW window field is available
    plugin_instance.window = nullptr;
    EXPECT_EQ(plugin_instance.window, nullptr);
#endif

#ifdef HAVE_SKIA
    // Test Skia surface field is available
    plugin_instance.surface = nullptr;
    EXPECT_EQ(plugin_instance.surface, nullptr);
#endif
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
