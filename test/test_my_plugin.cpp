#include <gtest/gtest.h>
#include "../my_plugin.h"

// Test to verify parameter functionality
TEST(MyPluginTest, ParameterTest) {
    // Test that parameter count is correct
    EXPECT_EQ(PARAM_COUNT, 5);
    
    // Test parameter ID values
    EXPECT_EQ(PARAM_DEPTH, 0);
    EXPECT_EQ(PARAM_SHARPNESS, 1);
    EXPECT_EQ(PARAM_SELECTIVITY, 2);
    EXPECT_EQ(PARAM_MODE, 3);
    EXPECT_EQ(PARAM_BALANCE, 4);
}

TEST(MyPluginTest, PlaceholderTest) {
    ASSERT_EQ(1, 1);
    // Replace with actual tests for your plugin
    // For example:
    // EXPECT_EQ(add(2, 2), 4);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
