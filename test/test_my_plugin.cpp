#include <gtest/gtest.h>
#include "../my_plugin.h"

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

// Test that the plugin with ImGui integration can be created and destroyed properly
TEST(MyPluginTest, PluginImGuiIntegration) {
    // Access the plugin factory
    extern const struct clap_plugin_factory my_plugin_factory;
    
    // Mock host (minimal implementation for testing)
    static const clap_host_t test_host = {
        CLAP_VERSION,
        nullptr,  // host_data
        "TestHost",  // name
        "Test",   // vendor
        "https://test.com",  // url
        "1.0.0",  // version
        nullptr,  // get_extension
        nullptr,  // request_restart
        nullptr,  // request_process
        nullptr,  // request_callback
    };
    
    // Create plugin instance
    const clap_plugin_t* plugin = my_plugin_factory.create_plugin(&my_plugin_factory, &test_host, "com.example.myplugin");
    ASSERT_NE(plugin, nullptr);
    
    // Initialize the plugin (this should create ImGui context)
    bool init_result = plugin->init(plugin);
    EXPECT_TRUE(init_result);
    
    // Verify plugin data structure
    my_plugin_t* self = (my_plugin_t*)plugin->plugin_data;
    ASSERT_NE(self, nullptr);
    EXPECT_NE(self->imgui_context, nullptr);  // ImGui context should be created
    
    // Call on_main_thread to exercise ImGui functionality
    plugin->on_main_thread(plugin);
    
    // Destroy the plugin (this should clean up ImGui context)
    plugin->destroy(plugin);
    
    // Free the plugin memory (normally done by host)
    free(self);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
