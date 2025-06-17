#include <gtest/gtest.h>
#include "../my_plugin.h"

TEST(MyPluginTest, PlaceholderTest) {
    ASSERT_EQ(1, 1);
}

TEST(MyPluginTest, GraphicsLibrariesIntegration) {
    // Test that graphics libraries are properly integrated at compile time
#ifdef HAVE_X11
    EXPECT_TRUE(true) << "X11 support is enabled";
#endif

#ifdef HAVE_SKIA
    EXPECT_TRUE(true) << "Skia support is enabled";
    // Note: Skia is optional and may not be enabled in all builds
#endif

    // Test the graphics backend info
    std::string backend_info = clap_jules::graphics::getGraphicsBackendInfo();
    EXPECT_FALSE(backend_info.empty()) << "Graphics backend info should not be empty";
    std::cout << "Graphics backend: " << backend_info << std::endl;
}

TEST(MyPluginTest, PluginStructureWithGraphics) {
    // Test that the plugin structure includes graphics fields
    my_plugin_t plugin_instance;
    
    // Initialize the basic plugin structure
    plugin_instance.plugin.desc = nullptr;
    plugin_instance.plugin.plugin_data = &plugin_instance;
    
    // Test GUI-related fields are available
    plugin_instance.gui_created = false;
    plugin_instance.gui_visible = false;
    plugin_instance.gui_width = 320;
    plugin_instance.gui_height = 240;
    plugin_instance.gui_api = nullptr;
    plugin_instance.gui_is_floating = false;
    plugin_instance.native_window = nullptr;
    plugin_instance.needs_redraw = false;
    
    EXPECT_EQ(plugin_instance.gui_created, false);
    EXPECT_EQ(plugin_instance.gui_visible, false);
    EXPECT_EQ(plugin_instance.gui_width, 320);
    EXPECT_EQ(plugin_instance.gui_height, 240);
    EXPECT_EQ(plugin_instance.gui_api, nullptr);
    EXPECT_EQ(plugin_instance.gui_is_floating, false);
    EXPECT_EQ(plugin_instance.native_window, nullptr);
    EXPECT_EQ(plugin_instance.needs_redraw, false);
}

TEST(MyPluginTest, GraphicsContextCreation) {
    // Test that graphics context can be created
    auto graphics = clap_jules::graphics::createGraphicsContext(320, 240);
    EXPECT_NE(graphics, nullptr) << "Graphics context should be created successfully";
    
    if (graphics) {
        EXPECT_EQ(graphics->getWidth(), 320);
        EXPECT_EQ(graphics->getHeight(), 240);
        
        // Test basic drawing operations don't crash
        graphics->clear(clap_jules::graphics::Color(50, 50, 50));
        graphics->drawRect(clap_jules::graphics::Rect(10, 10, 100, 50), 
                          clap_jules::graphics::Color(255, 100, 100));
        graphics->drawCircle(clap_jules::graphics::Point(160, 120), 30, 
                            clap_jules::graphics::Color(100, 255, 100));
        graphics->drawLine(clap_jules::graphics::Point(50, 200), 
                          clap_jules::graphics::Point(250, 200),
                          clap_jules::graphics::Color(100, 100, 255), 3.0f);
        graphics->drawText("Test", clap_jules::graphics::Point(50, 150), 
                          clap_jules::graphics::Color(255, 255, 255), 16.0f);
        graphics->present();
        
        // Test pixel data access
        const void* pixel_data = graphics->getPixelData();
        EXPECT_NE(pixel_data, nullptr) << "Pixel data should be accessible";
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
