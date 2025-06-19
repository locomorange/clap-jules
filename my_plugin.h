#pragma once

#include <clap/clap.h>
#include <memory>

#ifdef HAVE_GLFW
#include <clap/ext/gui.h>
#endif

// Forward declarations for graphics system
namespace clap_jules {
namespace graphics {
class GraphicsContext;
#if defined(__linux__) && defined(HAVE_X11)
class X11Renderer;
#endif
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
class Win32Renderer;
#endif
}
}

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    
#ifdef HAVE_GLFW
    // GUI-related data
    bool gui_created;
    bool gui_visible;
    uint32_t gui_width;
    uint32_t gui_height;
    const char* gui_api;
    bool gui_is_floating;
    bool needs_redraw;
    
    // Parent window for embedding
    void* native_window;
    
    // Graphics system
    std::unique_ptr<clap_jules::graphics::GraphicsContext> graphics_context;
    
#if defined(__linux__) && defined(HAVE_X11)
    std::unique_ptr<clap_jules::graphics::X11Renderer> x11_renderer;
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    std::unique_ptr<clap_jules::graphics::Win32Renderer> win32_renderer;
#endif
#endif
    
    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
