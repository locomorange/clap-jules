#pragma once

#include <clap/clap.h>

// Forward declaration
#if VSTGUI_ENABLED
class MyPluginEditor;
#endif

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
#if VSTGUI_ENABLED
    MyPluginEditor* gui_editor;
#endif
    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
