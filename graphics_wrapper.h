#pragma once

#ifdef HAVE_SKIA
#include <core/SkCanvas.h>
#include <core/SkPaint.h>
#include <core/SkBitmap.h>
#include <core/SkImageInfo.h>
#include <core/SkSurface.h>
#endif

#include <cstdint>
#include <memory>

namespace clap_jules {

/**
 * Simple graphics wrapper that provides basic drawing capabilities.
 * Uses Skia when available, falls back to simple implementation otherwise.
 */
class GraphicsContext {
public:
    GraphicsContext(int width, int height);
    ~GraphicsContext();
    
    // Basic drawing operations
    void clear(uint32_t color = 0xFF000000); // ARGB format
    void drawRect(int x, int y, int width, int height, uint32_t color);
    void drawCircle(int centerX, int centerY, int radius, uint32_t color);
    void drawLine(int x1, int y1, int x2, int y2, uint32_t color);
    
    // Get the raw pixel data (for plugin GUI rendering)
    const uint32_t* getPixels() const;
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    
    // Check if Skia is available
    static bool isSkiaAvailable();

private:
    int width_;
    int height_;
    
#ifdef HAVE_SKIA
    sk_sp<SkSurface> surface_;
    SkCanvas* canvas_;
#else
    std::unique_ptr<uint32_t[]> pixels_;
#endif
    
    void initializeGraphics();
    void cleanup();
};

} // namespace clap_jules