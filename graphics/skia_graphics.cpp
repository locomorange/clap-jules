#include "skia_graphics.h"
#include <vector>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <functional>

#if defined(__linux__) && defined(HAVE_X11)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <windows.h>
#include <wingdi.h>
#endif

namespace clap_jules {
namespace graphics {

// Simple software-based graphics context implementation
class SoftwareGraphicsContext : public GraphicsContext {
private:
    std::vector<uint32_t> pixel_buffer_;
    int width_, height_;
    
    uint32_t colorToUint32(const Color& color) {
        return (color.a << 24) | (color.r << 16) | (color.g << 8) | color.b;
    }
    
    void setPixel(int x, int y, const Color& color) {
        if (x >= 0 && x < width_ && y >= 0 && y < height_) {
            pixel_buffer_[y * width_ + x] = colorToUint32(color);
        }
    }
    
    void drawPixel(int x, int y, const Color& color) {
        if (x >= 0 && x < width_ && y >= 0 && y < height_) {
            uint32_t current = pixel_buffer_[y * width_ + x];
            uint32_t new_color = colorToUint32(color);
            
            if (color.a == 255) {
                pixel_buffer_[y * width_ + x] = new_color;
            } else {
                // Simple alpha blending
                uint8_t alpha = color.a;
                uint8_t inv_alpha = 255 - alpha;
                
                uint8_t r = ((new_color >> 16) & 0xFF) * alpha / 255 + ((current >> 16) & 0xFF) * inv_alpha / 255;
                uint8_t g = ((new_color >> 8) & 0xFF) * alpha / 255 + ((current >> 8) & 0xFF) * inv_alpha / 255;
                uint8_t b = (new_color & 0xFF) * alpha / 255 + (current & 0xFF) * inv_alpha / 255;
                uint8_t a = std::max((new_color >> 24) & 0xFF, (current >> 24) & 0xFF);
                
                pixel_buffer_[y * width_ + x] = (a << 24) | (r << 16) | (g << 8) | b;
            }
        }
    }

public:
    SoftwareGraphicsContext(int width, int height) 
        : width_(width), height_(height) {
        pixel_buffer_.resize(width * height, 0xFF000000); // Black with full alpha
    }
    
    void clear(const Color& color) override {
        uint32_t pixel = colorToUint32(color);
        std::fill(pixel_buffer_.begin(), pixel_buffer_.end(), pixel);
    }
    
    void drawRect(const Rect& rect, const Color& color) override {
        int x1 = static_cast<int>(rect.x);
        int y1 = static_cast<int>(rect.y);
        int x2 = static_cast<int>(rect.x + rect.width);
        int y2 = static_cast<int>(rect.y + rect.height);
        
        for (int y = y1; y < y2; y++) {
            for (int x = x1; x < x2; x++) {
                drawPixel(x, y, color);
            }
        }
    }
    
    void drawCircle(const Point& center, float radius, const Color& color) override {
        int cx = static_cast<int>(center.x);
        int cy = static_cast<int>(center.y);
        int r = static_cast<int>(radius);
        
        for (int y = cy - r; y <= cy + r; y++) {
            for (int x = cx - r; x <= cx + r; x++) {
                float dx = x - center.x;
                float dy = y - center.y;
                if (dx * dx + dy * dy <= radius * radius) {
                    drawPixel(x, y, color);
                }
            }
        }
    }
    
    void drawLine(const Point& start, const Point& end, const Color& color, float width) override {
        // Simple Bresenham line algorithm
        int x0 = static_cast<int>(start.x);
        int y0 = static_cast<int>(start.y);
        int x1 = static_cast<int>(end.x);
        int y1 = static_cast<int>(end.y);
        
        int dx = abs(x1 - x0);
        int dy = abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;
        
        int thick = static_cast<int>(width / 2);
        
        while (true) {
            // Draw thick line by drawing multiple pixels around the center
            for (int i = -thick; i <= thick; i++) {
                for (int j = -thick; j <= thick; j++) {
                    drawPixel(x0 + i, y0 + j, color);
                }
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
        // Simple bitmap font rendering - just draw rectangles for now
        int x = static_cast<int>(position.x);
        int y = static_cast<int>(position.y);
        int char_width = static_cast<int>(size * 0.6f);
        int char_height = static_cast<int>(size);
        
        for (size_t i = 0; i < text.length(); i++) {
            Rect char_rect(x + i * char_width, y, char_width - 1, char_height);
            drawRect(char_rect, color);
        }
    }
    
    void present() override {
        // Nothing to do for software rendering - pixel buffer is always current
    }
    
    const void* getPixelData() const override {
        return pixel_buffer_.data();
    }
    
    int getWidth() const override {
        return width_;
    }
    
    int getHeight() const override {
        return height_;
    }
};

#if defined(__linux__) && defined(HAVE_X11)
class X11RendererImpl : public X11Renderer {
private:
    Display* display_;
    Window window_;
    XImage* image_;
    GC gc_;
    bool initialized_;
    int width_, height_;
    
public:
    X11RendererImpl() : display_(nullptr), window_(0), image_(nullptr), gc_(0), initialized_(false), width_(0), height_(0) {}
    
    ~X11RendererImpl() {
        if (initialized_) {
            if (image_) {
                XDestroyImage(image_);
            }
            if (gc_) {
                XFreeGC(display_, gc_);
            }
        }
    }
    
    bool initialize(unsigned long window_id, int width, int height) override {
        display_ = XOpenDisplay(nullptr);
        if (!display_) return false;
        
        window_ = static_cast<Window>(window_id);
        width_ = width;
        height_ = height;
        
        gc_ = XCreateGC(display_, window_, 0, nullptr);
        if (!gc_) return false;
        
        initialized_ = true;
        return true;
    }
    
    bool isInitialized() const override {
        return initialized_;
    }
    
    bool presentPixelBuffer(const uint32_t* pixels, int width, int height) override {
        if (!initialized_ || !pixels) return false;
        
        // Create XImage from pixel buffer
        char* data = reinterpret_cast<char*>(const_cast<uint32_t*>(pixels));
        image_ = XCreateImage(display_, DefaultVisual(display_, DefaultScreen(display_)),
                             24, ZPixmap, 0, data, width, height, 32, width * 4);
        
        if (!image_) return false;
        
        XPutImage(display_, window_, gc_, image_, 0, 0, 0, 0, width, height);
        XFlush(display_);
        
        return true;
    }
    
    void resize(int width, int height) override {
        width_ = width;
        height_ = height;
        
        if (image_) {
            XDestroyImage(image_);
            image_ = nullptr;
        }
    }
    
    void invalidate() override {
        if (initialized_) {
            XClearWindow(display_, window_);
            XFlush(display_);
        }
    }
};
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
class Win32RendererImpl : public Win32Renderer {
private:
    HWND hwnd_;
    HDC hdc_;
    HBITMAP bitmap_;
    BITMAPINFO bitmap_info_;
    bool initialized_;
    int width_, height_;
    std::function<void()> redraw_callback_;
    
public:
    Win32RendererImpl() : hwnd_(nullptr), hdc_(nullptr), bitmap_(nullptr), initialized_(false), width_(0), height_(0) {
        memset(&bitmap_info_, 0, sizeof(bitmap_info_));
    }
    
    ~Win32RendererImpl() {
        if (bitmap_) {
            DeleteObject(bitmap_);
        }
        if (hdc_) {
            ReleaseDC(hwnd_, hdc_);
        }
    }
    
    bool initialize(void* hwnd, int width, int height) override {
        hwnd_ = static_cast<HWND>(hwnd);
        if (!hwnd_) return false;
        
        hdc_ = GetDC(hwnd_);
        if (!hdc_) return false;
        
        width_ = width;
        height_ = height;
        
        // Setup bitmap info
        bitmap_info_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmap_info_.bmiHeader.biWidth = width;
        bitmap_info_.bmiHeader.biHeight = -height; // Top-down DIB
        bitmap_info_.bmiHeader.biPlanes = 1;
        bitmap_info_.bmiHeader.biBitCount = 32;
        bitmap_info_.bmiHeader.biCompression = BI_RGB;
        
        initialized_ = true;
        return true;
    }
    
    bool isInitialized() const override {
        return initialized_;
    }
    
    bool presentPixelBuffer(const uint32_t* pixels, int width, int height) override {
        if (!initialized_ || !pixels) return false;
        
        SetDIBitsToDevice(hdc_, 0, 0, width, height, 0, 0, 0, height,
                         pixels, &bitmap_info_, DIB_RGB_COLORS);
        
        return true;
    }
    
    void resize(int width, int height) override {
        width_ = width;
        height_ = height;
        
        bitmap_info_.bmiHeader.biWidth = width;
        bitmap_info_.bmiHeader.biHeight = -height;
        
        if (bitmap_) {
            DeleteObject(bitmap_);
            bitmap_ = nullptr;
        }
    }
    
    void invalidate() override {
        if (initialized_ && redraw_callback_) {
            redraw_callback_();
        }
        
        if (hwnd_) {
            InvalidateRect(hwnd_, nullptr, TRUE);
        }
    }
    
    void setRedrawCallback(std::function<void()> callback) override {
        redraw_callback_ = callback;
    }
};
#endif

// Factory functions
std::unique_ptr<GraphicsContext> createGraphicsContext(int width, int height) {
    return std::make_unique<SoftwareGraphicsContext>(width, height);
}

std::string getGraphicsBackendInfo() {
    return "Software Renderer with Platform Integration";
}

bool isSkiaAvailable() {
    // For now, we don't have Skia integrated, so return false
    // This can be updated when Skia is properly integrated
    return false;
}

#if defined(__linux__) && defined(HAVE_X11)
std::unique_ptr<X11Renderer> createX11Renderer() {
    return std::make_unique<X11RendererImpl>();
}
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
std::unique_ptr<Win32Renderer> createWin32Renderer() {
    return std::make_unique<Win32RendererImpl>();
}
#endif

} // namespace graphics
} // namespace clap_jules