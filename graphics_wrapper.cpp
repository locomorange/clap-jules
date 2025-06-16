#include "graphics_wrapper.h"
#include <cstring>
#include <cstdlib>
#include <algorithm>

namespace clap_jules {

GraphicsContext::GraphicsContext(int width, int height) 
    : width_(width), height_(height) {
    initializeGraphics();
}

GraphicsContext::~GraphicsContext() {
    cleanup();
}

void GraphicsContext::initializeGraphics() {
#ifdef HAVE_SKIA
    // Create Skia surface
    SkImageInfo info = SkImageInfo::Make(width_, height_, kBGRA_8888_SkColorType, kPremul_SkAlphaType);
    surface_ = SkSurface::MakeRaster(info);
    if (surface_) {
        canvas_ = surface_->getCanvas();
    } else {
        // Fallback to software rendering if Skia surface creation fails
        pixels_ = std::make_unique<uint32_t[]>(width_ * height_);
        std::fill_n(pixels_.get(), width_ * height_, 0xFF000000);
    }
#else
    // Software rendering fallback
    pixels_ = std::make_unique<uint32_t[]>(width_ * height_);
    std::fill_n(pixels_.get(), width_ * height_, 0xFF000000);
#endif
}

void GraphicsContext::cleanup() {
#ifdef HAVE_SKIA
    canvas_ = nullptr;
    surface_.reset();
#endif
    pixels_.reset();
}

void GraphicsContext::clear(uint32_t color) {
#ifdef HAVE_SKIA
    if (canvas_) {
        SkColor skColor = SkColorSetARGB(
            (color >> 24) & 0xFF,  // A
            (color >> 16) & 0xFF,  // R
            (color >> 8) & 0xFF,   // G
            color & 0xFF           // B
        );
        canvas_->clear(skColor);
        return;
    }
#endif
    
    // Software fallback
    if (pixels_) {
        std::fill_n(pixels_.get(), width_ * height_, color);
    }
}

void GraphicsContext::drawRect(int x, int y, int width, int height, uint32_t color) {
#ifdef HAVE_SKIA
    if (canvas_) {
        SkPaint paint;
        paint.setColor(SkColorSetARGB(
            (color >> 24) & 0xFF,  // A
            (color >> 16) & 0xFF,  // R
            (color >> 8) & 0xFF,   // G
            color & 0xFF           // B
        ));
        paint.setStyle(SkPaint::kFill_Style);
        
        SkRect rect = SkRect::MakeXYWH(x, y, width, height);
        canvas_->drawRect(rect, paint);
        return;
    }
#endif
    
    // Software fallback
    if (!pixels_) return;
    
    int x1 = std::max(0, x);
    int y1 = std::max(0, y);
    int x2 = std::min(width_, x + width);
    int y2 = std::min(height_, y + height);
    
    for (int py = y1; py < y2; ++py) {
        for (int px = x1; px < x2; ++px) {
            pixels_[py * width_ + px] = color;
        }
    }
}

void GraphicsContext::drawCircle(int centerX, int centerY, int radius, uint32_t color) {
#ifdef HAVE_SKIA
    if (canvas_) {
        SkPaint paint;
        paint.setColor(SkColorSetARGB(
            (color >> 24) & 0xFF,  // A
            (color >> 16) & 0xFF,  // R
            (color >> 8) & 0xFF,   // G
            color & 0xFF           // B
        ));
        paint.setStyle(SkPaint::kFill_Style);
        
        canvas_->drawCircle(centerX, centerY, radius, paint);
        return;
    }
#endif
    
    // Software fallback - simple circle drawing
    if (!pixels_) return;
    
    int radiusSquared = radius * radius;
    int x1 = std::max(0, centerX - radius);
    int y1 = std::max(0, centerY - radius);
    int x2 = std::min(width_, centerX + radius + 1);
    int y2 = std::min(height_, centerY + radius + 1);
    
    for (int py = y1; py < y2; ++py) {
        for (int px = x1; px < x2; ++px) {
            int dx = px - centerX;
            int dy = py - centerY;
            if (dx * dx + dy * dy <= radiusSquared) {
                pixels_[py * width_ + px] = color;
            }
        }
    }
}

void GraphicsContext::drawLine(int x1, int y1, int x2, int y2, uint32_t color) {
#ifdef HAVE_SKIA
    if (canvas_) {
        SkPaint paint;
        paint.setColor(SkColorSetARGB(
            (color >> 24) & 0xFF,  // A
            (color >> 16) & 0xFF,  // R
            (color >> 8) & 0xFF,   // G
            color & 0xFF           // B
        ));
        paint.setStrokeWidth(1);
        paint.setStyle(SkPaint::kStroke_Style);
        
        canvas_->drawLine(x1, y1, x2, y2, paint);
        return;
    }
#endif
    
    // Software fallback - simple line drawing using Bresenham's algorithm
    if (!pixels_) return;
    
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;
    
    int x = x1, y = y1;
    while (true) {
        if (x >= 0 && x < width_ && y >= 0 && y < height_) {
            pixels_[y * width_ + x] = color;
        }
        
        if (x == x2 && y == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

const uint32_t* GraphicsContext::getPixels() const {
#ifdef HAVE_SKIA
    if (surface_) {
        // Get pixels from Skia surface
        SkPixmap pixmap;
        if (surface_->peekPixels(&pixmap)) {
            return static_cast<const uint32_t*>(pixmap.addr());
        }
    }
#endif
    
    return pixels_.get();
}

bool GraphicsContext::isSkiaAvailable() {
#ifdef HAVE_SKIA
    return true;
#else
    return false;
#endif
}

} // namespace clap_jules