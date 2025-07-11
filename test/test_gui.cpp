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
        const clap_plugin_factory* factory = &my_plugin_factory;
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
    
    // GUI extension should always be available with the new graphics abstraction
    EXPECT_NE(gui, nullptr) << "GUI extension should be available";
}

TEST_F(GUITest, GuiApiSupport) {
    const clap_plugin_gui_t* gui = (const clap_plugin_gui_t*)plugin->get_extension(plugin, CLAP_EXT_GUI);
    ASSERT_NE(gui, nullptr);
    
    // Should support embedded windows (the preferred mode)
    EXPECT_TRUE(gui->is_api_supported(plugin, CLAP_WINDOW_API_X11, false)) << "Should support X11 embedded windows";
    EXPECT_TRUE(gui->is_api_supported(plugin, CLAP_WINDOW_API_WIN32, false)) << "Should support Win32 embedded windows";
    EXPECT_TRUE(gui->is_api_supported(plugin, CLAP_WINDOW_API_COCOA, false)) << "Should support Cocoa embedded windows";
    
    // Should also support floating windows
    EXPECT_TRUE(gui->is_api_supported(plugin, CLAP_WINDOW_API_WAYLAND, true)) << "Should support Wayland floating windows";
    
    // Test preferred API
    const char* preferred_api;
    bool is_floating;
    EXPECT_TRUE(gui->get_preferred_api(plugin, &preferred_api, &is_floating));
    EXPECT_FALSE(is_floating) << "Should prefer embedded windows";
    EXPECT_NE(preferred_api, nullptr) << "Should return a preferred API";
}

TEST_F(GUITest, GuiSize) {
    const clap_plugin_gui_t* gui = (const clap_plugin_gui_t*)plugin->get_extension(plugin, CLAP_EXT_GUI);
    ASSERT_NE(gui, nullptr);
    
    uint32_t width, height;
    EXPECT_TRUE(gui->get_size(plugin, &width, &height));
    EXPECT_GT(width, 0) << "Width should be positive";
    EXPECT_GT(height, 0) << "Height should be positive";
    EXPECT_EQ(width, 320) << "Default width should be 320";
    EXPECT_EQ(height, 240) << "Default height should be 240";
}

TEST_F(GUITest, GuiResizeCapabilities) {
    const clap_plugin_gui_t* gui = (const clap_plugin_gui_t*)plugin->get_extension(plugin, CLAP_EXT_GUI);
    ASSERT_NE(gui, nullptr);
    
    // Our new implementation supports resizing
    EXPECT_TRUE(gui->can_resize(plugin)) << "Should support resizing";
    
    clap_gui_resize_hints_t hints;
    EXPECT_TRUE(gui->get_resize_hints(plugin, &hints)) << "Should provide resize hints";
    
    uint32_t width = 500, height = 400;
    EXPECT_TRUE(gui->adjust_size(plugin, &width, &height)) << "Should support size adjustment";
    EXPECT_TRUE(gui->set_size(plugin, width, height)) << "Should support size setting";
}

TEST_F(GUITest, PluginStateInitialization) {
    my_plugin_t* plugin_data = (my_plugin_t*)plugin->plugin_data;
    
    // Test GUI state initialization
    EXPECT_FALSE(plugin_data->gui_created) << "GUI should not be created initially";
    EXPECT_FALSE(plugin_data->gui_visible) << "GUI should not be visible initially";
    EXPECT_EQ(plugin_data->gui_width, 320) << "GUI width should be initialized";
    EXPECT_EQ(plugin_data->gui_height, 240) << "GUI height should be initialized";
    EXPECT_EQ(plugin_data->gui_api, nullptr) << "GUI API should be null initially";
    EXPECT_FALSE(plugin_data->gui_is_floating) << "GUI should not be floating initially";
    EXPECT_EQ(plugin_data->native_window, nullptr) << "Native window should be null initially";
    EXPECT_TRUE(plugin_data->needs_redraw) << "Should need redraw initially";
}