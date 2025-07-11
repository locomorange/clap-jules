#include "skia_graphics.h"
#include <cstdio>
#include <vector>
#include <cmath>
#include <cstring>

namespace clap_jules {
namespace graphics {

// Simple software-based graphics context implementation
class SoftwareGraphicsContext : public GraphicsContext {
private:
    int width_, height_;
    std::vector<uint32_t> pixels_;
    
public:
    SoftwareGraphicsContext(int width, int height) 
        : width_(width), height_(height), pixels_(width * height, 0xFF404040) {}
    
    void clear(const Color& color) override {
        uint32_t pixel = (color.a << 24) | (color.r << 16) | (color.g << 8) | color.b;
        std::fill(pixels_.begin(), pixels_.end(), pixel);
    }
    
    void drawRect(const Rect& rect, const Color& color) override {
        uint32_t pixel = (color.a << 24) | (color.r << 16) | (color.g << 8) | color.b;
        
        int x1 = std::max(0, (int)rect.x);
        int y1 = std::max(0, (int)rect.y);
        int x2 = std::min(width_, (int)(rect.x + rect.width));
        int y2 = std::min(height_, (int)(rect.y + rect.height));
        
        for (int y = y1; y < y2; y++) {
            for (int x = x1; x < x2; x++) {
                if (x >= 0 && x < width_ && y >= 0 && y < height_) {
                    pixels_[y * width_ + x] = pixel;
                }
            }
        }
    }
    
    void drawCircle(const Point& center, float radius, const Color& color) override {
        uint32_t pixel = (color.a << 24) | (color.r << 16) | (color.g << 8) | color.b;
        
        int cx = (int)center.x;
        int cy = (int)center.y;
        int r = (int)radius;
        
        for (int y = cy - r; y <= cy + r; y++) {
            for (int x = cx - r; x <= cx + r; x++) {
                if (x >= 0 && x < width_ && y >= 0 && y < height_) {
                    float dx = x - center.x;
                    float dy = y - center.y;
                    if (dx * dx + dy * dy <= radius * radius) {
                        pixels_[y * width_ + x] = pixel;
                    }
                }
            }
        }
    }
    
    void drawLine(const Point& start, const Point& end, const Color& color, float width) override {
        uint32_t pixel = (color.a << 24) | (color.r << 16) | (color.g << 8) | color.b;
        
        // Simple Bresenham-like line drawing
        int x0 = (int)start.x, y0 = (int)start.y;
        int x1 = (int)end.x, y1 = (int)end.y;
        
        int dx = abs(x1 - x0);
        int dy = abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;
        
        while (true) {
            if (x0 >= 0 && x0 < width_ && y0 >= 0 && y0 < height_) {
                pixels_[y0 * width_ + x0] = pixel;
            }
            
            if (x0 == x1 && y0 == y1) break;
            
            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y0 += sy;
            }
        }
    }
    
    void drawText(const std::string& text, const Point& position, const Color& color, float size) override {
        // Simple text rendering - just draw rectangles for now
        uint32_t pixel = (color.a << 24) | (color.r << 16) | (color.g << 8) | color.b;
        
        int x = (int)position.x;
        int y = (int)position.y;
        int char_width = std::max(1, (int)(size * 0.6f));
        int char_height = (int)size;
        
        for (size_t i = 0; i < text.length(); i++) {
            // Draw a simple rectangle for each character
            for (int dy = 0; dy < char_height; dy++) {
                for (int dx = 0; dx < char_width; dx++) {
                    int px = x + i * char_width + dx;
                    int py = y + dy;
                    if (px >= 0 && px < width_ && py >= 0 && py < height_) {
                        pixels_[py * width_ + px] = pixel;
                    }
                }
            }
        }
    }
    
    void present() override {
        // Nothing needed for software rendering
    }
    
    const void* getPixelData() const override {
        return pixels_.data();
    }
    
    int getWidth() const override {
        return width_;
    }
    
    int getHeight() const override {
        return height_;
    }
};

std::unique_ptr<GraphicsContext> createGraphicsContext(int width, int height) {
    return std::make_unique<SoftwareGraphicsContext>(width, height);
}

std::string getGraphicsBackendInfo() {
    return "Software Renderer (Cross-platform)";
}

bool isSkiaAvailable() {
    return false; // For now, we're using software rendering only
}

} // namespace graphics
} // namespace clap_jules