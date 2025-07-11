#pragma once

#include <memory>
#include <string>
#include <functional>

namespace clap_jules {
namespace graphics {

// Color structure for simple rendering
struct Color {
    int r, g, b, a;
    Color(int red = 0, int green = 0, int blue = 0, int alpha = 255) 
        : r(red), g(green), b(blue), a(alpha) {}
};

// Point structure
struct Point {
    float x, y;
    Point(float x_val = 0, float y_val = 0) : x(x_val), y(y_val) {}
};

// Rectangle structure
struct Rect {
    float x, y, width, height;
    Rect(float x_val = 0, float y_val = 0, float w = 0, float h = 0) 
        : x(x_val), y(y_val), width(w), height(h) {}
};

// Abstract graphics context for cross-platform rendering
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

// Factory functions
std::unique_ptr<GraphicsContext> createGraphicsContext(int width, int height);
std::string getGraphicsBackendInfo();
bool isSkiaAvailable();

} // namespace graphics
} // namespace clap_jules