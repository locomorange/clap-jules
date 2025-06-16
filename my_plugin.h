#pragma once

#include <clap/clap.h>
#include <clap/ext/gui.h>
#include <memory>
#include "graphics/skia_graphics.h"

#if defined(__linux__) && defined(HAVE_X11)
#include "graphics/x11_renderer.h"
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include "graphics/win32_renderer.h"
#endif

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    
    // Host reference for callbacks
    const clap_host_t* host = nullptr;
    
    // GUI-related data
    bool gui_created = false;
    bool gui_visible = false;
    uint32_t gui_width = 0;
    uint32_t gui_height = 0;
    const char* gui_api = nullptr;
    bool gui_is_floating = false;
    
    // Graphics context for rendering
    std::unique_ptr<clap_jules::graphics::GraphicsContext> graphics_context = nullptr;
    
    // Platform-specific window handle
    void* native_window = nullptr;
    
    // Platform-specific renderer
#if defined(__linux__) && defined(HAVE_X11)
    std::unique_ptr<clap_jules::graphics::X11Renderer> x11_renderer = nullptr;
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    std::unique_ptr<clap_jules::graphics::Win32Renderer> win32_renderer = nullptr;
#endif
    
    // Render callback data
    bool needs_redraw = false;
    
    // VU meter data
    float current_level_left = 0.0f;  // Current audio level for left channel (0.0 to 1.0)
    float current_level_right = 0.0f; // Current audio level for right channel (0.0 to 1.0)
    float peak_level_left = 0.0f;     // Peak level for left channel
    float peak_level_right = 0.0f;    // Peak level for right channel
    int peak_hold_counter_left = 0;   // Counter for peak hold display
    int peak_hold_counter_right = 0;  // Counter for peak hold display
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
