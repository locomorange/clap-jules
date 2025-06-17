#include <gtest/gtest.h>
#include "../gui.h"
#include "../my_plugin.h"
#include <GLFW/glfw3.h>

// Test that GLFW initializes properly
TEST(GuiTest, GLFWInitialization) {
    // In headless environments, GLFW may fail to initialize
    // This is expected behavior, so we make this a conditional test
    bool result = gui_init();
    if (result) {
        gui_cleanup();
        SUCCEED() << "GLFW initialized successfully";
    } else {
        GTEST_SKIP() << "GLFW initialization failed (likely headless environment)";
    }
}

// Test basic renderer functionality
TEST(GuiTest, RendererCreation) {
    // Skip this test in headless environments
    if (!glfwInit()) {
        GTEST_SKIP() << "GLFW initialization failed (likely headless environment)";
        return;
    }
    
    // Create an invisible window for testing
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(400, 300, "Test", NULL, NULL);
    if (!window) {
        glfwTerminate();
        GTEST_SKIP() << "Could not create GLFW window (likely headless environment)";
        return;
    }
    
    simple_renderer_t renderer;
    ASSERT_TRUE(renderer_init(&renderer, window, 400, 300));
    
    // Test basic rendering operations (these should not crash)
    renderer_begin_frame(&renderer);
    renderer_clear(&renderer, 0.2f, 0.2f, 0.2f);
    renderer_draw_rect(&renderer, 10, 10, 50, 30, 1.0f, 0.0f, 0.0f, 1.0f);
    renderer_end_frame(&renderer);
    
    renderer_cleanup(&renderer);
    glfwDestroyWindow(window);
    glfwTerminate();
}

// Test GUI extension constants
TEST(GuiTest, CLAPConstants) {
    // Verify CLAP GUI constants are available
    ASSERT_STREQ(CLAP_EXT_GUI, "clap.gui");
    
    // Verify window API constants
    ASSERT_STREQ(CLAP_WINDOW_API_WIN32, "win32");
    ASSERT_STREQ(CLAP_WINDOW_API_X11, "x11");
    ASSERT_STREQ(CLAP_WINDOW_API_COCOA, "cocoa");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}