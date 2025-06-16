#include <gtest/gtest.h>

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
