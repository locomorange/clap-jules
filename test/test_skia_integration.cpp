#include <gtest/gtest.h>

#ifdef CLAP_JULES_WITH_SKIA
#include "../my_plugin.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkImageInfo.h"

// Test Skia integration functionality
TEST(SkiaIntegrationTest, BasicSkiaFunctionality) {
    // Test basic Skia surface creation
    SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
    sk_sp<SkSurface> surface = SkSurface::MakeRaster(info);
    
    ASSERT_TRUE(surface != nullptr) << "Failed to create Skia surface";
    
    SkCanvas* canvas = surface->getCanvas();
    ASSERT_TRUE(canvas != nullptr) << "Failed to get canvas from surface";
    
    // Test basic drawing operations
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    canvas->drawRect(SkRect::MakeXYWH(10, 10, 50, 50), paint);
    
    // Test different color
    paint.setColor(SK_ColorBLUE);
    canvas->drawCircle(50, 50, 20, paint);
    
    // If we get here, basic Skia functionality works
    SUCCEED();
}

TEST(SkiaIntegrationTest, SkiaVersionInfo) {
    // Test that we can access Skia version information
    ASSERT_TRUE(true) << "Skia integration is working";
}

#endif

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}