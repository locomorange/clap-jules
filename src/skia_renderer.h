#pragma once

#ifdef CLAP_JULES_HAS_SKIA
#include <include/core/SkSurface.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/gpu/GrDirectContext.h>
#include <include/gpu/gl/GrGLInterface.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#endif

#include <cstdint>

namespace clap_jules {

// Forward declarations
struct SkiaRenderer;

// Skia rendering interface
class SkiaRenderer {
public:
    SkiaRenderer();
    ~SkiaRenderer();
    
    // Initialize Skia with OpenGL context
    bool initialize(int width, int height);
    void cleanup();
    
    // Rendering methods
    void beginFrame();
    void endFrame();
    void resize(int width, int height);
    
    // Basic drawing methods
    void clear(float r, float g, float b, float a = 1.0f);
    void drawRect(float x, float y, float width, float height, uint32_t color);
    void drawText(const char* text, float x, float y, float size, uint32_t color);
    
    // Get canvas for custom drawing
#ifdef CLAP_JULES_HAS_SKIA
    SkCanvas* getCanvas();
#endif
    
    bool isInitialized() const { return initialized_; }
    
private:
    bool initialized_;
    int width_, height_;
    
#ifdef CLAP_JULES_HAS_SKIA
    sk_sp<GrDirectContext> grContext_;
    sk_sp<SkSurface> surface_;
    sk_sp<const GrGLInterface> interface_;
#endif
};

} // namespace clap_jules