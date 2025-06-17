# Skia Integration Guide

This document explains how to integrate Google's Skia graphics library into the clap-jules plugin.

## Current Implementation

The current implementation uses a simple OpenGL-based renderer (`renderer.h/cpp`) that provides:
- Basic 2D drawing primitives (rectangles, text placeholders)
- OpenGL context management
- Viewport scaling and resizing

## Skia Integration Options

### Option 1: vcpkg (Recommended for Development)

```bash
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh

# Install Skia with OpenGL support
./vcpkg/vcpkg install skia[gl]

# Update CMakeLists.txt
find_package(skia CONFIG REQUIRED)
target_link_libraries(my_clap_plugin PRIVATE skia skia::skia)
```

### Option 2: Prebuilt Binaries

1. Download Skia prebuilt binaries from Google's builds
2. Extract to a libs/skia directory
3. Update CMakeLists.txt to link against the prebuilt libraries

### Option 3: Build from Source

Follow the [official Skia build guide](https://skia.org/docs/user/build/).

## Code Integration Steps

### 1. Update renderer.h

```cpp
#pragma once

#ifdef USE_SKIA
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#endif

typedef struct {
    struct GLFWwindow* window;
    uint32_t width;
    uint32_t height;
    bool initialized;
    
#ifdef USE_SKIA
    sk_sp<GrDirectContext> grContext;
    sk_sp<SkSurface> surface;
    SkCanvas* canvas;
#endif
} simple_renderer_t;
```

### 2. Update renderer.cpp

```cpp
#ifdef USE_SKIA
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/GrBackendSurface.h"
#endif

bool renderer_init(simple_renderer_t* renderer, GLFWwindow* window, uint32_t width, uint32_t height) {
    // ... existing OpenGL setup ...
    
#ifdef USE_SKIA
    // Create Skia OpenGL context
    auto interface = GrGLMakeNativeInterface();
    renderer->grContext = GrDirectContext::MakeGL(interface);
    
    // Create Skia surface
    GrGLFramebufferInfo fbInfo;
    fbInfo.fFBOID = 0; // Default framebuffer
    fbInfo.fFormat = GL_RGBA8;
    
    GrBackendRenderTarget backendRT(width, height, 0, 0, fbInfo);
    renderer->surface = SkSurface::MakeFromBackendRenderTarget(
        renderer->grContext.get(), backendRT, kBottomLeft_GrSurfaceOrigin, 
        kRGBA_8888_SkColorType, nullptr, nullptr);
    
    renderer->canvas = renderer->surface->getCanvas();
#endif
    
    return true;
}

void renderer_draw_rect(simple_renderer_t* renderer, float x, float y, float width, float height, 
                       float r, float g, float b, float a) {
#ifdef USE_SKIA
    SkPaint paint;
    paint.setColor(SkColorSetARGB(a*255, r*255, g*255, b*255));
    renderer->canvas->drawRect(SkRect::MakeXYWH(x, y, width, height), paint);
#else
    // Fallback to OpenGL implementation
    // ... existing code ...
#endif
}
```

### 3. Update CMakeLists.txt

```cmake
option(USE_SKIA "Use Skia for graphics rendering" OFF)

if(USE_SKIA)
    find_package(skia CONFIG REQUIRED)
    target_compile_definitions(my_clap_plugin PRIVATE USE_SKIA)
    target_link_libraries(my_clap_plugin PRIVATE skia skia::skia)
endif()
```

### 4. Build with Skia

```bash
cmake . -B build -DUSE_SKIA=ON
cmake --build build --config Release
```

## Benefits of Skia Integration

- **Advanced 2D Graphics**: Bezier curves, gradients, complex shapes
- **Text Rendering**: Full Unicode support, font management
- **Image Processing**: Filters, effects, image manipulation  
- **Hardware Acceleration**: GPU-accelerated rendering
- **Cross-Platform**: Consistent rendering across all platforms

## Migration Strategy

1. **Phase 1**: Keep current OpenGL renderer as fallback
2. **Phase 2**: Add Skia as optional dependency
3. **Phase 3**: Implement Skia renderer alongside OpenGL
4. **Phase 4**: Make Skia the default renderer
5. **Phase 5**: Remove OpenGL fallback (optional)

This approach ensures the plugin continues to work during the transition.