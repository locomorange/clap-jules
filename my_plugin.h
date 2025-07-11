#pragma once

#include <clap/clap.h>

#ifdef HAS_X11_GUI
#include <X11/Xlib.h>
#endif

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    
    // GUI related members
    #ifdef HAS_X11_GUI
    Display* display;
    Window window;
    bool gui_created;
    bool gui_visible;
    uint32_t gui_width;
    uint32_t gui_height;
    #endif
    
    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
