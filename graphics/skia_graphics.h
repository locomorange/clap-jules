#pragma once

#include <string>
#include <memory>
#include <cstdint>
#include <functional>

namespace clap_jules {
namespace graphics {

// Basic color structure
struct Color {
    uint8_t r, g, b, a;
    
    Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
};

// Basic point structure
struct Point {
    float x, y;
    
    Point(float x_pos, float y_pos) : x(x_pos), y(y_pos) {}
};

// Basic rectangle structure
struct Rect {
    float x, y, width, height;
    
    Rect(float x_pos, float y_pos, float w, float h)
        : x(x_pos), y(y_pos), width(w), height(h) {}
};

// Graphics context interface
class GraphicsContext {
public:
    virtual ~GraphicsContext() = default;
    
    virtual void clear(const Color& color) = 0;
    virtual void drawRect(const Rect& rect, const Color& color) = 0;
    virtual void drawCircle(const Point& center, float radius, const Color& color) = 0;
    virtual void drawLine(const Point& start, const Point& end, const Color& color, float width = 1.0f) = 0;
    virtual void drawText(const std::string& text, const Point& position, const Color& color, float size = 12.0f) = 0;
    
    virtual void present() = 0;
    virtual const void* getPixelData() const = 0;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
};

// Platform-specific renderers
#if defined(__linux__) && defined(HAVE_X11)
class X11Renderer {
public:
    X11Renderer() = default;
    virtual ~X11Renderer() = default;
    
    virtual bool initialize(unsigned long window_id, int width, int height) = 0;
    virtual bool isInitialized() const = 0;
    virtual bool presentPixelBuffer(const uint32_t* pixels, int width, int height) = 0;
    virtual void resize(int width, int height) = 0;
    virtual void invalidate() = 0;
};
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
class Win32Renderer {
public:
    Win32Renderer() = default;
    virtual ~Win32Renderer() = default;
    
    virtual bool initialize(void* hwnd, int width, int height) = 0;
    virtual bool isInitialized() const = 0;
    virtual bool presentPixelBuffer(const uint32_t* pixels, int width, int height) = 0;
    virtual void resize(int width, int height) = 0;
    virtual void invalidate() = 0;
    virtual void setRedrawCallback(std::function<void()> callback) = 0;
};
#endif

// Factory functions
std::unique_ptr<GraphicsContext> createGraphicsContext(int width, int height);
std::string getGraphicsBackendInfo();
bool isSkiaAvailable();

#if defined(__linux__) && defined(HAVE_X11)
std::unique_ptr<X11Renderer> createX11Renderer();
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
std::unique_ptr<Win32Renderer> createWin32Renderer();
#endif

} // namespace graphics
} // namespace clap_jules