#pragma once

#include <clap/clap.h>

// Forward declarations for Qt GUI
class PluginWidget;

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    const clap_host_t *host;
    PluginWidget *gui_widget;
    bool gui_created;
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
