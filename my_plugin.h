#pragma once

#include <clap/clap.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations for Qt GUI (only for C++)
#ifdef __cplusplus
class PluginWidget;
#else
typedef void PluginWidget;
#endif

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    const clap_host_t *host;
    PluginWidget *gui_widget;
    bool gui_created;
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;

#ifdef __cplusplus
}
#endif
