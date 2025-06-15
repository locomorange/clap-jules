#include "graphics_renderer.h"
#include <vector>
#include <cstring>
#include <cstdio>

namespace clap_jules {

class BasicGraphicsRenderer : public GraphicsRenderer {
private:
    std::vector<uint32_t> framebuffer_;
    int width_, height_;
    
public:
    BasicGraphicsRenderer() : width_(0), height_(0) {}
    
    void clear(uint32_t color) override {
        std::fill(framebuffer_.begin(), framebuffer_.end(), color);
    }
    
    void drawRect(float x, float y, float width, float height, uint32_t color) override {
        int ix = static_cast<int>(x);
        int iy = static_cast<int>(y);
        int iw = static_cast<int>(width);
        int ih = static_cast<int>(height);
        
        for (int py = iy; py < iy + ih && py < height_; ++py) {
            for (int px = ix; px < ix + iw && px < width_; ++px) {
                if (px >= 0 && py >= 0) {
                    framebuffer_[py * width_ + px] = color;
                }
            }
        }
    }
    
    void drawCircle(float x, float y, float radius, uint32_t color) override {
        int cx = static_cast<int>(x);
        int cy = static_cast<int>(y);
        int r = static_cast<int>(radius);
        
        for (int py = cy - r; py <= cy + r; ++py) {
            for (int px = cx - r; px <= cx + r; ++px) {
                if (px >= 0 && px < width_ && py >= 0 && py < height_) {
                    int dx = px - cx;
                    int dy = py - cy;
                    if (dx * dx + dy * dy <= r * r) {
                        framebuffer_[py * width_ + px] = color;
                    }
                }
            }
        }
    }
    
    void drawText(const char* text, float x, float y, uint32_t color) override {
        // Simple text rendering placeholder - would use Skia for proper text rendering
        // Note: In a production plugin, logging should be done through proper audio plugin logging
        // For now, we'll just draw a placeholder rectangle representing the text
        
        // Draw a simple placeholder rectangle for text
        int textWidth = static_cast<int>(strlen(text)) * 8; // Approximate character width
        drawRect(x, y, static_cast<float>(textWidth), 16.0f, color);
    }
    
    void beginFrame(int width, int height) override {
        width_ = width;
        height_ = height;
        framebuffer_.resize(width * height);
        clear(0xFF000000); // Clear to black
    }
    
    void endFrame() override {
        // Frame rendering complete - in a real implementation, this would
        // present the frame to the display system
    }
    
    const void* getFrameData() const override {
        return framebuffer_.data();
    }
    
    size_t getFrameSize() const override {
        return framebuffer_.size() * sizeof(uint32_t);
    }
};

#ifdef HAVE_SKIA
// This would be the Skia implementation
#include "core/SkCanvas.h"
#include "core/SkSurface.h"
#include "core/SkPaint.h"

class SkiaGraphicsRenderer : public GraphicsRenderer {
private:
    sk_sp<SkSurface> surface_;
    SkCanvas* canvas_;
    std::vector<uint8_t> pixelData_;
    int width_, height_;
    
public:
    SkiaGraphicsRenderer() : canvas_(nullptr), width_(0), height_(0) {}
    
    void clear(uint32_t color) override {
        if (canvas_) {
            canvas_->clear(color);
        }
    }
    
    void drawRect(float x, float y, float width, float height, uint32_t color) override {
        if (canvas_) {
            SkPaint paint;
            paint.setColor(color);
            canvas_->drawRect(SkRect::MakeXYWH(x, y, width, height), paint);
        }
    }
    
    void drawCircle(float x, float y, float radius, uint32_t color) override {
        if (canvas_) {
            SkPaint paint;
            paint.setColor(color);
            canvas_->drawCircle(x, y, radius, paint);
        }
    }
    
    void drawText(const char* text, float x, float y, uint32_t color) override {
        if (canvas_) {
            SkPaint paint;
            paint.setColor(color);
            paint.setTextSize(16);
            canvas_->drawString(text, x, y, paint);
        }
    }
    
    void beginFrame(int width, int height) override {
        width_ = width;
        height_ = height;
        
        SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
        surface_ = SkSurface::MakeRaster(info);
        if (surface_) {
            canvas_ = surface_->getCanvas();
        }
    }
    
    void endFrame() override {
        if (surface_) {
            SkImageInfo info = SkImageInfo::MakeN32Premul(width_, height_);
            pixelData_.resize(width_ * height_ * 4);
            surface_->readPixels(info, pixelData_.data(), width_ * 4, 0, 0);
        }
    }
    
    const void* getFrameData() const override {
        return pixelData_.data();
    }
    
    size_t getFrameSize() const override {
        return pixelData_.size();
    }
};
#endif // HAVE_SKIA

std::unique_ptr<GraphicsRenderer> createGraphicsRenderer() {
#ifdef HAVE_SKIA
    return std::make_unique<SkiaGraphicsRenderer>();
#else
    return std::make_unique<BasicGraphicsRenderer>();
#endif
}

} // namespace clap_jules