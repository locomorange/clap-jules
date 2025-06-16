#pragma once

#include <clap/clap.h>
#include <clap/ext/gui.h>
#include "graphics_wrapper.h"

// Forward declarations for platform-specific types
#ifdef _WIN32
    #include <windows.h>
    typedef HWND platform_window_t;
#elif defined(__APPLE__)
    typedef void* platform_window_t; // NSView*
#else
    #include <X11/Xlib.h>
    typedef Window platform_window_t;
    typedef Display* platform_display_t;
#endif

// GUI state structure
typedef struct {
    bool is_created;
    bool is_visible;
    bool is_floating;
    const char* api;
    uint32_t width;
    uint32_t height;
    double scale;
    
    // Platform-specific window handles
#ifdef _WIN32
    HWND hwnd;
    HWND parent_hwnd;
#elif defined(__APPLE__)
    platform_window_t nsview;
    platform_window_t parent_nsview;
#else
    platform_display_t display;
    platform_window_t window;
    platform_window_t parent_window;
#endif
    
    // Graphics context for rendering
    clap_jules::GraphicsContext* graphics;
} gui_state_t;

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    gui_state_t gui;
    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
