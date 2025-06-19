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
    
    // Host reference
    const clap_host_t* host;
    
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
    
    // Rendering state
    bool needs_redraw;
#endif
    
    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
