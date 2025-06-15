#pragma once

#include <clap/clap.h>
#include <clap/ext/params.h>

// Parameter IDs
enum {
    PARAM_DEPTH = 0,
    PARAM_SHARPNESS,
    PARAM_SELECTIVITY,
    PARAM_MODE,
    PARAM_BALANCE,
    PARAM_COUNT
};

// Forward declaration
#if VSTGUI_ENABLED
class MyPluginEditor;
#endif

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    const clap_host_t* host;
#if VSTGUI_ENABLED
    MyPluginEditor* gui_editor;
#endif
    // Parameter values
    double params[PARAM_COUNT];
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
