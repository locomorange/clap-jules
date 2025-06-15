#pragma once

#include <clap/clap.h>

// Forward declaration
#if VSTGUI_ENABLED
class MyPluginEditor;
#endif
class MyPluginParameterManager;

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    const clap_host_t* host;
#if VSTGUI_ENABLED
    MyPluginEditor* gui_editor;
#endif
    MyPluginParameterManager* parameter_manager;
    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
