#pragma once

#include <clap/clap.h>
#include <clap/ext/gui.h>
#include <clap/ext/params.h>
#include <memory>
#include "graphics/skia_graphics.h"
#include "spectrum_analyzer.h"

#if defined(__linux__) && defined(HAVE_X11)
#include "graphics/x11_renderer.h"
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include "graphics/win32_renderer.h"
#endif

// Parameter IDs
enum {
    PARAM_VISUALIZATION_TYPE = 0,
    PARAM_COUNT
};

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    
    // Audio processing
    double sample_rate = 44100.0;
    bool is_processing = false;
    
    // Spectrum analyzer
    std::unique_ptr<clap_jules::audio::SpectrumAnalyzer> spectrum_analyzer = nullptr;
    
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
    
    // Parameters
    std::atomic<int> visualization_type_param = 0; // 0=Lines, 1=Dots, 2=Bins, 3=Fill
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
