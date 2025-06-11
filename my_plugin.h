#pragma once

#include <clap/clap.h>
#include "imgui.h"

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
