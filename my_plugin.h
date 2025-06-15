#pragma once

#include <clap/clap.h>
#include <clap/ext/params.h>

// Forward declaration
#if VSTGUI_ENABLED
class MyPluginEditor;
#endif

// Parameter IDs for the soothe2-style interface
enum {
    PARAM_DEPTH = 0,
    PARAM_SHARPNESS,
    PARAM_SELECTIVITY,
    PARAM_MODE,
    PARAM_BALANCE,
    PARAM_LINK,
    PARAM_FREQUENCY,
    PARAM_GAIN,
    PARAM_Q,
    PARAM_BYPASS,
    PARAM_COUNT
};

// Basic plugin structure
struct my_plugin_t {
    clap_plugin_t plugin;
    const clap_host_t* host;
#if VSTGUI_ENABLED
    MyPluginEditor* gui_editor;
#endif
    // Parameter values storage
    double param_values[PARAM_COUNT];
};

// External access to parameter information
extern const clap_param_info_t param_infos[PARAM_COUNT];

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
