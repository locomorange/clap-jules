#pragma once

#include <clap/clap.h>
#include <imgui.h>

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    // ImGui context for the plugin instance
    ImGuiContext* imgui_context;

    // GUI related state
    bool gui_created;
    bool gui_is_visible;
    clap_window_t parent_window; // To store parent window details

    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;

// Function to get the plugin factory (for cross-platform compatibility)
CLAP_EXPORT const struct clap_plugin_factory* get_plugin_factory();
