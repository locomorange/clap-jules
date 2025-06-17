#include "skia_renderer.h"
#include <cstdio>

#ifdef CLAP_JULES_HAS_GLFW
// Let GLFW handle OpenGL headers for cross-platform compatibility
#include <GLFW/glfw3.h>
#endif

namespace clap_jules {

SkiaRenderer::SkiaRenderer() 
    : initialized_(false), width_(0), height_(0) {
}

SkiaRenderer::~SkiaRenderer() {
    cleanup();
}

bool SkiaRenderer::initialize(int width, int height) {
    if (initialized_) {
        return true;
    }
    
    width_ = width;
    height_ = height;
    
#ifdef CLAP_JULES_HAS_SKIA
    // Initialize Skia with OpenGL backend
    interface_ = GrGLMakeNativeInterface();
    if (!interface_) {
        printf("SkiaRenderer: Failed to create GL interface\n");
        return false;
    }
    
    grContext_ = GrDirectContext::MakeGL(interface_);
    if (!grContext_) {
        printf("SkiaRenderer: Failed to create GrDirectContext\n");
        return false;
    }
    
    // Create framebuffer surface
    GrGLFramebufferInfo framebufferInfo;
    framebufferInfo.fFBOID = 0; // assume default framebuffer
    framebufferInfo.fFormat = GL_RGBA8;
    
    SkColorType colorType = kRGBA_8888_SkColorType;
    GrBackendRenderTarget backendRenderTarget(width_, height_, 0, 0, framebufferInfo);
    
    surface_ = SkSurfaces::WrapBackendRenderTarget(
        grContext_.get(),
        backendRenderTarget,
        kBottomLeft_GrSurfaceOrigin,
        colorType,
        nullptr,
        nullptr
    );
    
    if (!surface_) {
        printf("SkiaRenderer: Failed to create SkSurface\n");
        grContext_.reset();
        return false;
    }
    
    printf("SkiaRenderer: Initialized successfully (%dx%d)\n", width_, height_);
#else
    printf("SkiaRenderer: Skia not available, using fallback rendering\n");
#endif
    
    initialized_ = true;
    return true;
}

void SkiaRenderer::cleanup() {
    if (!initialized_) {
        return;
    }
    
#ifdef CLAP_JULES_HAS_SKIA
    surface_.reset();
    grContext_.reset();
    interface_.reset();
#endif
    
    initialized_ = false;
    printf("SkiaRenderer: Cleaned up\n");
}

void SkiaRenderer::beginFrame() {
    if (!initialized_) {
        return;
    }
    
#ifdef CLAP_JULES_HAS_SKIA
    if (grContext_) {
        grContext_->resetContext();
    }
#endif
}

void SkiaRenderer::endFrame() {
    if (!initialized_) {
        return;
    }
    
#ifdef CLAP_JULES_HAS_SKIA
    if (surface_) {
        surface_->getCanvas()->flush();
    }
    if (grContext_) {
        grContext_->flush();
    }
#endif
}

void SkiaRenderer::resize(int width, int height) {
    if (width == width_ && height == height_) {
        return;
    }
    
    width_ = width;
    height_ = height;
    
    if (initialized_) {
        cleanup();
        initialize(width_, height_);
    }
}

void SkiaRenderer::clear(float r, float g, float b, float a) {
#ifdef CLAP_JULES_HAS_SKIA
    if (surface_) {
        SkCanvas* canvas = surface_->getCanvas();
        canvas->clear(SkColorSetARGB(
            (uint8_t)(a * 255),
            (uint8_t)(r * 255), 
            (uint8_t)(g * 255), 
            (uint8_t)(b * 255)
        ));
        return;
    }
#endif
    
    // Fallback to OpenGL
#ifdef CLAP_JULES_HAS_GLFW
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
#endif
}

void SkiaRenderer::drawRect(float x, float y, float width, float height, uint32_t color) {
#ifdef CLAP_JULES_HAS_SKIA
    if (surface_) {
        SkCanvas* canvas = surface_->getCanvas();
        SkPaint paint;
        paint.setColor(color);
        canvas->drawRect(SkRect::MakeXYWH(x, y, width, height), paint);
        return;
    }
#endif
    
    // Fallback rendering could be implemented here
    printf("SkiaRenderer: drawRect fallback not implemented\n");
}

void SkiaRenderer::drawText(const char* text, float x, float y, float size, uint32_t color) {
#ifdef CLAP_JULES_HAS_SKIA
    if (surface_) {
        SkCanvas* canvas = surface_->getCanvas();
        SkPaint paint;
        paint.setColor(color);
        paint.setTextSize(size);
        canvas->drawString(text, x, y, SkFont(), paint);
        return;
    }
#endif
    
    // Fallback rendering could be implemented here
    printf("SkiaRenderer: drawText fallback not implemented\n");
}

#ifdef CLAP_JULES_HAS_SKIA
SkCanvas* SkiaRenderer::getCanvas() {
    if (surface_) {
        return surface_->getCanvas();
    }
    return nullptr;
}
#endif

} // namespace clap_jules