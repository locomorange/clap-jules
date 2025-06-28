#pragma once

#include <clap/clap.h>

#ifdef __cplusplus
extern "C" {
#endif

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    // GUI-related data
    void* gui_window;
    bool gui_visible;
    void* parent_window;
    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;

#ifdef __cplusplus
}
#endif
