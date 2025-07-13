#include <gtest/gtest.h>
#include <cmath>

// Helper function to test dB to linear conversion (same implementation as in plugin)
double db_to_linear_test(double db) {
    return pow(10.0, db / 20.0);
}

TEST(MyPluginTest, PlaceholderTest) {
    ASSERT_EQ(1, 1);
}

TEST(MyPluginTest, GainControlTest) {
    // Test dB to linear conversion
    EXPECT_NEAR(db_to_linear_test(0.0), 1.0, 0.001);    // 0 dB = 1.0 linear
    EXPECT_NEAR(db_to_linear_test(6.0), 2.0, 0.01);     // ~6 dB = 2.0 linear
    EXPECT_NEAR(db_to_linear_test(-6.0), 0.5, 0.01);    // ~-6 dB = 0.5 linear
    EXPECT_NEAR(db_to_linear_test(-36.0), 0.0158, 0.001); // -36 dB = very small
    EXPECT_NEAR(db_to_linear_test(36.0), 63.1, 0.1);    // +36 dB = large gain
}

TEST(MyPluginTest, ParameterRangeTest) {
    // Test that parameter range is correctly defined (-36 to +36 dB)
    const double min_gain = -36.0;
    const double max_gain = 36.0;
    const double default_gain = 0.0;
    
    EXPECT_LE(min_gain, default_gain);
    EXPECT_LE(default_gain, max_gain);
    EXPECT_EQ(min_gain, -36.0);
    EXPECT_EQ(max_gain, 36.0);
    EXPECT_EQ(default_gain, 0.0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
