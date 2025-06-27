#pragma once

#include <clap/clap.h>
#include <sys/types.h>  // For pid_t

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    
    // GUI state
    bool gui_created;
    bool gui_visible;
    pid_t flutter_pid;  // Process ID of Flutter app
    
    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
