#include <gtest/gtest.h>
#include "graphics/skia_graphics.h"

using namespace clap_jules::graphics;

class GraphicsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test graphics context
        graphics = createGraphicsContext(100, 100);
    }
    
    std::unique_ptr<GraphicsContext> graphics;
};

TEST_F(GraphicsTest, ContextCreation) {
    ASSERT_NE(graphics.get(), nullptr);
}

TEST_F(GraphicsTest, BackendInfo) {
    std::string info = getGraphicsBackendInfo();
    EXPECT_FALSE(info.empty());
    
    // Should contain either "Skia" or "Stub"
    EXPECT_TRUE(info.find("Skia") != std::string::npos || 
                info.find("Stub") != std::string::npos);
}

TEST_F(GraphicsTest, SkiaAvailability) {
    bool available = isSkiaAvailable();
    // Should be consistent with the backend info
    std::string info = getGraphicsBackendInfo();
    if (available) {
        EXPECT_NE(info.find("Skia"), std::string::npos);
    } else {
        EXPECT_NE(info.find("Stub"), std::string::npos);
    }
}

TEST_F(GraphicsTest, BasicDrawingOperations) {
    // These should not crash and should be callable
    EXPECT_NO_THROW({
        graphics->clear(Color(255, 0, 0));
        graphics->drawRect(Rect(10, 10, 50, 30), Color(0, 255, 0));
        graphics->drawCircle(Point(50, 50), 20, Color(0, 0, 255));
        graphics->drawLine(Point(0, 0), Point(99, 99), Color(255, 255, 0), 2.0f);
        graphics->drawText("Test", Point(25, 75), Color(255, 255, 255), 12.0f);
    });
}

TEST_F(GraphicsTest, StateManagement) {
    // These should not crash
    EXPECT_NO_THROW({
        graphics->save();
        graphics->translate(10, 20);
        graphics->scale(1.5f, 1.5f);
        graphics->rotate(0.5f);
        graphics->restore();
    });
}

TEST_F(GraphicsTest, ColorValues) {
    Color red(255, 0, 0);
    EXPECT_EQ(red.r, 255);
    EXPECT_EQ(red.g, 0);
    EXPECT_EQ(red.b, 0);
    EXPECT_EQ(red.a, 255); // Default alpha
    
    Color transparent(100, 150, 200, 128);
    EXPECT_EQ(transparent.a, 128);
}

TEST_F(GraphicsTest, GeometryStructs) {
    Point p(10.5f, 20.3f);
    EXPECT_FLOAT_EQ(p.x, 10.5f);
    EXPECT_FLOAT_EQ(p.y, 20.3f);
    
    Rect r(5, 10, 100, 200);
    EXPECT_FLOAT_EQ(r.x, 5);
    EXPECT_FLOAT_EQ(r.y, 10);
    EXPECT_FLOAT_EQ(r.width, 100);
    EXPECT_FLOAT_EQ(r.height, 200);
}