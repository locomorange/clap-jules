#include <gtest/gtest.h>
#include "../my_plugin.h"
#include <clap/ext/gui.h>

// Mock host structure for testing
static const clap_host_t test_host = {
    CLAP_VERSION,
    nullptr, // host_data
    "Test Host", // name
    "Test Vendor", // vendor
    "https://test.com", // url
    "1.0.0", // version
    nullptr, // get_extension
    nullptr, // request_restart
    nullptr, // request_process
    nullptr, // request_callback
};

TEST(MyPluginTest, PlaceholderTest) {
    ASSERT_EQ(1, 1);
    // Replace with actual tests for your plugin
    // For example:
    // EXPECT_EQ(add(2, 2), 4);
}

TEST(MyPluginTest, BasicPluginCreation) {
    // Test that we can create a plugin instance
    const clap_plugin_t *plugin = my_plugin_factory.create_plugin(
        &my_plugin_factory, &test_host, "com.example.myplugin");
    
    ASSERT_NE(plugin, nullptr);
    EXPECT_STREQ(plugin->desc->id, "com.example.myplugin");
    EXPECT_STREQ(plugin->desc->name, "My First CLAP Plugin");
    
    // Initialize the plugin
    EXPECT_TRUE(plugin->init(plugin));
    
    // Test GUI extension
    const clap_plugin_gui_t *gui_ext = 
        (const clap_plugin_gui_t *)plugin->get_extension(plugin, CLAP_EXT_GUI);
    ASSERT_NE(gui_ext, nullptr) << "GUI extension should be available";
    
    // Test API support
#ifdef _WIN32
    EXPECT_TRUE(gui_ext->is_api_supported(plugin, CLAP_WINDOW_API_WIN32, false));
#elif defined(__APPLE__)
    EXPECT_TRUE(gui_ext->is_api_supported(plugin, CLAP_WINDOW_API_COCOA, false));
#elif defined(__linux__)
    EXPECT_TRUE(gui_ext->is_api_supported(plugin, CLAP_WINDOW_API_X11, false));
#endif
    
    // Test preferred API
    const char *preferred_api;
    bool is_floating;
    EXPECT_TRUE(gui_ext->get_preferred_api(plugin, &preferred_api, &is_floating));
    EXPECT_NE(preferred_api, nullptr);
    EXPECT_FALSE(is_floating); // Should prefer embedded windows
    
    // Test GUI creation
    EXPECT_TRUE(gui_ext->create(plugin, preferred_api, false));
    
    // Test size functions
    uint32_t width, height;
    EXPECT_TRUE(gui_ext->get_size(plugin, &width, &height));
    EXPECT_EQ(width, 400U); // Default width
    EXPECT_EQ(height, 300U); // Default height
    
    // Test resizing
    EXPECT_TRUE(gui_ext->can_resize(plugin));
    EXPECT_TRUE(gui_ext->set_size(plugin, 500, 400));
    EXPECT_TRUE(gui_ext->get_size(plugin, &width, &height));
    EXPECT_EQ(width, 500U);
    EXPECT_EQ(height, 400U);
    
    // Test show/hide
    EXPECT_TRUE(gui_ext->show(plugin));
    EXPECT_TRUE(gui_ext->hide(plugin));
    
    // Clean up
    gui_ext->destroy(plugin);
    plugin->destroy(plugin);
}

#ifdef CLAP_JULES_WITH_SKIA
TEST(MyPluginTest, SkiaIntegrationTest) {
    // Test that Skia integration is enabled
    // This test only runs when Skia is enabled
    ASSERT_TRUE(true) << "Skia integration is enabled";
}
#else
TEST(MyPluginTest, SkiaDisabledTest) {
    // Test that runs when Skia is disabled
    ASSERT_TRUE(true) << "Skia integration is disabled";
}
#endif

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
