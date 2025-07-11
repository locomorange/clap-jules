#pragma once

#include <clap/clap.h>

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    // Add any other plugin-specific data here
    bool ui_visible;
    void* flutter_process;  // For tracking Flutter UI process
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;

// Flutter UI integration functions
bool launch_flutter_ui(my_plugin_t* plugin);
void close_flutter_ui(my_plugin_t* plugin);
