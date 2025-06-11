#include <gtest/gtest.h>
#include <imgui.h>

// Simple test to verify imgui integration
TEST(ImGuiIntegrationTest, ImGuiHeadersAccessible) {
    // Test that we can access imgui functions and constants
    EXPECT_TRUE(IMGUI_VERSION_NUM > 0);
    
    // Test that we can call basic imgui functions
    // Note: We don't create a context here since this is just a header accessibility test
    const char* version = ImGui::GetVersion();
    EXPECT_NE(version, nullptr);
    EXPECT_GT(strlen(version), 0);
}

// Test that imgui can be initialized (basic context creation)
TEST(ImGuiIntegrationTest, ImGuiContextCreation) {
    ImGuiContext* ctx = ImGui::CreateContext();
    EXPECT_NE(ctx, nullptr);
    
    if (ctx) {
        ImGui::DestroyContext(ctx);
    }
}