#pragma once

#include <clap/clap.h>
#include <clap/ext/gui.h>

// Forward declarations for GLFW
struct GLFWwindow;

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    
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
    
    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
