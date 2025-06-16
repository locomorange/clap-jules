#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace clap_jules {
namespace graphics {

// Forward declarations
struct Color {
    uint8_t r, g, b, a;
    Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0, uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
};

struct Point {
    float x, y;
    Point(float x_val = 0.0f, float y_val = 0.0f) : x(x_val), y(y_val) {}
};

struct Rect {
    float x, y, width, height;
    Rect(float x_val = 0.0f, float y_val = 0.0f, float w = 0.0f, float h = 0.0f)
        : x(x_val), y(y_val), width(w), height(h) {}
};

// Abstract graphics interface - can be implemented with Skia or other backends
class GraphicsContext {
public:
    virtual ~GraphicsContext() = default;
    
    // Basic drawing operations
    virtual void clear(const Color& color) = 0;
    virtual void drawRect(const Rect& rect, const Color& color) = 0;
    virtual void drawCircle(const Point& center, float radius, const Color& color) = 0;
    virtual void drawLine(const Point& start, const Point& end, const Color& color, float width = 1.0f) = 0;
    virtual void drawText(const std::string& text, const Point& position, const Color& color, float size = 12.0f) = 0;
    
    // State management
    virtual void save() = 0;
    virtual void restore() = 0;
    virtual void translate(float dx, float dy) = 0;
    virtual void scale(float sx, float sy) = 0;
    virtual void rotate(float radians) = 0;
};

// Factory function to create graphics context
std::unique_ptr<GraphicsContext> createGraphicsContext(int width, int height);

// Utility functions
bool isSkiaAvailable();
std::string getGraphicsBackendInfo();

} // namespace graphics
} // namespace clap_jules