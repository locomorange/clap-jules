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

// CLAP entry point
extern CLAP_EXPORT const clap_plugin_entry_t clap_entry;
