#include <gtest/gtest.h>
#include "../my_plugin.h"

// Test for GUI extension availability and basic functionality
class GUITest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a minimal host
        host_instance.host.clap_version = CLAP_VERSION;
        host_instance.host.host_data = &host_instance;
        host_instance.host.name = "Test Host";
        host_instance.host.vendor = "Test";
        host_instance.host.url = "http://test.com";
        host_instance.host.version = "1.0.0";
        host_instance.host.get_extension = [](const clap_host_t* host, const char* extension_id) -> const void* {
            return nullptr;
        };
        host_instance.host.request_restart = [](const clap_host_t* host) {};
        host_instance.host.request_process = [](const clap_host_t* host) {};
        host_instance.host.request_callback = [](const clap_host_t* host) {};
        
        // Create plugin instance
        const clap_plugin_factory* factory = get_plugin_factory();
        plugin = factory->create_plugin(factory, &host_instance.host, "com.example.myplugin");
        ASSERT_NE(plugin, nullptr);
        
        // Initialize plugin
        ASSERT_TRUE(plugin->init(plugin));
    }
    
    void TearDown() override {
        if (plugin) {
            plugin->destroy(plugin);
        }
    }
    
    struct test_host {
        clap_host_t host;
    };
    
    test_host host_instance;
    const clap_plugin_t* plugin = nullptr;
};

TEST_F(GUITest, GuiExtensionAvailable) {
    // Test that GUI extension is available
    const clap_plugin_gui_t* gui = (const clap_plugin_gui_t*)plugin->get_extension(plugin, CLAP_EXT_GUI);
    
#ifdef HAVE_GLFW
    EXPECT_NE(gui, nullptr) << "GUI extension should be available when GLFW is compiled in";
#else
    EXPECT_EQ(gui, nullptr) << "GUI extension should not be available when GLFW is not compiled in";
#endif
}

TEST_F(GUITest, GuiApiSupport) {
    const clap_plugin_gui_t* gui = (const clap_plugin_gui_t*)plugin->get_extension(plugin, CLAP_EXT_GUI);
    
#ifdef HAVE_GLFW
    ASSERT_NE(gui, nullptr);
    
    // Should support floating windows
    EXPECT_TRUE(gui->is_api_supported(plugin, "", true)) << "Should support floating windows";
    
    // Should not support embedded windows (we only implement floating)
    EXPECT_FALSE(gui->is_api_supported(plugin, "", false)) << "Should not support embedded windows";
    
    // Test preferred API
    const char* preferred_api;
    bool is_floating;
    EXPECT_TRUE(gui->get_preferred_api(plugin, &preferred_api, &is_floating));
    EXPECT_TRUE(is_floating) << "Should prefer floating windows";
#endif
}

TEST_F(GUITest, GuiSize) {
    const clap_plugin_gui_t* gui = (const clap_plugin_gui_t*)plugin->get_extension(plugin, CLAP_EXT_GUI);
    
#ifdef HAVE_GLFW
    ASSERT_NE(gui, nullptr);
    
    uint32_t width, height;
    EXPECT_TRUE(gui->get_size(plugin, &width, &height));
    EXPECT_GT(width, 0) << "Width should be positive";
    EXPECT_GT(height, 0) << "Height should be positive";
    EXPECT_EQ(width, 400) << "Default width should be 400";
    EXPECT_EQ(height, 300) << "Default height should be 300";
#endif
}

TEST_F(GUITest, GuiResizeCapabilities) {
    const clap_plugin_gui_t* gui = (const clap_plugin_gui_t*)plugin->get_extension(plugin, CLAP_EXT_GUI);
    
#ifdef HAVE_GLFW
    ASSERT_NE(gui, nullptr);
    
    // Our implementation has fixed size
    EXPECT_FALSE(gui->can_resize(plugin)) << "Should not support resizing";
    
    clap_gui_resize_hints_t hints;
    EXPECT_FALSE(gui->get_resize_hints(plugin, &hints)) << "Should not provide resize hints";
    
    uint32_t width = 500, height = 400;
    EXPECT_FALSE(gui->adjust_size(plugin, &width, &height)) << "Should not support size adjustment";
    EXPECT_FALSE(gui->set_size(plugin, width, height)) << "Should not support size setting";
#endif
}

TEST_F(GUITest, PluginStateInitialization) {
    my_plugin_t* plugin_data = (my_plugin_t*)plugin->plugin_data;
    
#ifdef HAVE_GLFW
    EXPECT_FALSE(plugin_data->gui_created) << "GUI should not be created initially";
    EXPECT_TRUE(plugin_data->button_red) << "Button should start red";
    EXPECT_EQ(plugin_data->window_width, 400) << "Window width should be initialized";
    EXPECT_EQ(plugin_data->window_height, 300) << "Window height should be initialized";
    EXPECT_EQ(plugin_data->window, nullptr) << "Window should be null initially";
#endif
}