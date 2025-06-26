#pragma once

#include <clap/clap.h>

#ifdef HAVE_GLFW
#include <clap/ext/gui.h>
// Forward declarations for GLFW
struct GLFWwindow;
#endif

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    
#ifdef HAVE_GLFW
    // GUI-related data
    struct GLFWwindow* window;
    bool gui_created;
    bool gui_visible;
    uint32_t gui_width;
    uint32_t gui_height;
    const char* gui_api;
    bool is_floating;
    
    // Parent window for embedding
    clap_window_t parent_window;
    
    // Minimal rendering state for window refresh
    bool needs_refresh;

    // Small draggable window
    struct GLFWwindow* small_window;
    int small_window_x;
    int small_window_y;
    bool is_dragging_small_window;
    double drag_offset_x; // Original offset within the small window where dragging started
    double drag_offset_y; // Original offset within the small window where dragging started

    // New fields for native drag handling
    int initial_drag_screen_x;       // Mouse cursor's screen X at drag start
    int initial_drag_screen_y;       // Mouse cursor's screen Y at drag start
    int initial_small_window_relative_x; // Small window's X relative to parent at drag start
    int initial_small_window_relative_y; // Small window's Y relative to parent at drag start

#if defined(_WIN32)
    HWND small_hwnd; // Store small window's native handle for Windows
#elif defined(__linux__)
    // These types will be defined by X11/Xlib.h included in my_plugin.cpp BEFORE this header
    struct _XDisplay* x11_display;       // Store X11 display (opaque struct)
    unsigned long small_x11_window; // Store small window's native handle for X11 (XID is unsigned long)
#endif

#endif
    
    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
