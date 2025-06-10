#pragma once

#include <clap/clap.h>
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"
#include <clap/ext/gui.h>

// Forward declaration for GLFWwindow
struct GLFWwindow;

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    ImGuiContext* imgui_context;
    GLFWwindow* window; // Store GLFW window

    // CLAP GUI Extension
    clap_plugin_gui_t plugin_gui_extension;
    bool is_gui_visible;
    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
