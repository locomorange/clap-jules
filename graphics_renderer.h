#pragma once

#include <memory>
#include <cstdint>

namespace clap_jules {

// Basic graphics context interface for plugin GUI rendering
class GraphicsRenderer {
public:
    virtual ~GraphicsRenderer() = default;
    
    // Basic drawing operations
    virtual void clear(uint32_t color) = 0;
    virtual void drawRect(float x, float y, float width, float height, uint32_t color) = 0;
    virtual void drawCircle(float x, float y, float radius, uint32_t color) = 0;
    virtual void drawText(const char* text, float x, float y, uint32_t color) = 0;
    
    // Canvas operations
    virtual void beginFrame(int width, int height) = 0;
    virtual void endFrame() = 0;
    
    // Get rendered data (for display or testing)
    virtual const void* getFrameData() const = 0;
    virtual size_t getFrameSize() const = 0;
};

// Factory function to create graphics renderer
std::unique_ptr<GraphicsRenderer> createGraphicsRenderer();

} // namespace clap_jules