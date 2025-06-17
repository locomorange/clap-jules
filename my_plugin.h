#pragma once

#include <clap/clap.h>
#include <clap/ext/gui.h>

#ifdef CLAP_JULES_HAS_GLFW
#include <GLFW/glfw3.h>
#endif

// GUI state structure
typedef struct {
    bool is_created;
    bool is_visible;
    const char* api;
    bool is_floating;
    uint32_t width;
    uint32_t height;
    double scale;
#ifdef CLAP_JULES_HAS_GLFW
    GLFWwindow* window;
#endif
    void* parent_window;
} my_plugin_gui_t;

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    // GUI state
    my_plugin_gui_t gui;
    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
