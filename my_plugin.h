#pragma once

#include <clap/clap.h>
#include <clap/ext/gui.h>

// Forward declaration for GUI data
typedef struct gui_data gui_data_t;

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    
    // GUI-related data
    gui_data_t *gui_data;
} my_plugin_t;

// Plugin factory ID
extern CLAP_EXPORT const struct clap_plugin_factory my_plugin_factory;

// Plugin descriptor (defined in plugin_factory.cpp)
extern const clap_plugin_descriptor_t my_plugin_descriptor;

// Plugin instance creation function
extern const clap_plugin_t *create_my_plugin_instance(const clap_host_t *host, const char *plugin_id);

// CLAP entry point
extern CLAP_EXPORT const clap_plugin_entry_t clap_entry;
