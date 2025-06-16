#pragma once

#include <clap/clap.h>
#include <clap/ext/gui.h>
#include "graphics/skia_graphics.h"

#if defined(__linux__) && defined(HAVE_X11)
#include "graphics/x11_renderer.h"
#endif

#ifdef _WIN32
#include "graphics/win32_renderer.h"
#endif

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    
    // GUI-related data
    bool gui_created;
    bool gui_visible;
    uint32_t gui_width;
    uint32_t gui_height;
    const char* gui_api;
    bool gui_is_floating;
    
    // Graphics context for rendering
    std::unique_ptr<clap_jules::graphics::GraphicsContext> graphics_context;
    
    // Platform-specific window handle
    void* native_window;
    
    // Platform-specific renderer
#if defined(__linux__) && defined(HAVE_X11)
    std::unique_ptr<clap_jules::graphics::X11Renderer> x11_renderer;
#endif

#ifdef _WIN32
    std::unique_ptr<clap_jules::graphics::Win32Renderer> win32_renderer;
#endif
    
    // Render callback data
    bool needs_redraw;
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
