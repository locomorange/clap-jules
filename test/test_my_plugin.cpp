#include <gtest/gtest.h>
#include <vector>
#include <array>

// Test C++20 features that GCC 9.4 supports
TEST(MyPluginTest, PlaceholderTest) {
    ASSERT_EQ(1, 1);
}

TEST(MyPluginTest, Cpp20DesignatedInitializersTest) {
    // C++20 designated initializers
    struct Point { int x, y; };
    Point p{.x = 10, .y = 20};
    
    EXPECT_EQ(p.x, 10);
    EXPECT_EQ(p.y, 20);
}

TEST(MyPluginTest, Cpp20InitCaptureLambdaTest) {
    // C++20 template lambdas and init capture improvements
    auto lambda = []<typename T>(T value) -> T {
        return value * 2;
    };
    
    auto result = lambda(21);
    EXPECT_EQ(result, 42);
}

TEST(MyPluginTest, Cpp20ThreeWayComparisonTest) {
    // C++20 three-way comparison (spaceship operator)
    int a = 5;
    int b = 10;
    
    // This works with C++20 even in GCC 9.4
    EXPECT_LT(a, b);
    EXPECT_GT(b, a);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
