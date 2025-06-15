#pragma once

#include <clap/clap.h>
#include <clap/ext/params.h>

// Forward declaration
#if VSTGUI_ENABLED
class MyPluginEditor;
#endif

// EQ Parameter IDs
enum EQParamIDs {
    // Low Band
    PARAM_LOW_FREQ = 0,
    PARAM_LOW_GAIN,
    PARAM_LOW_Q,
    PARAM_LOW_TYPE,
    PARAM_LOW_BYPASS,
    
    // Low-Mid Band
    PARAM_LOW_MID_FREQ,
    PARAM_LOW_MID_GAIN,
    PARAM_LOW_MID_Q,
    PARAM_LOW_MID_TYPE,
    PARAM_LOW_MID_BYPASS,
    
    // High-Mid Band
    PARAM_HIGH_MID_FREQ,
    PARAM_HIGH_MID_GAIN,
    PARAM_HIGH_MID_Q,
    PARAM_HIGH_MID_TYPE,
    PARAM_HIGH_MID_BYPASS,
    
    // High Band
    PARAM_HIGH_FREQ,
    PARAM_HIGH_GAIN,
    PARAM_HIGH_Q,
    PARAM_HIGH_TYPE,
    PARAM_HIGH_BYPASS,
    
    // Master
    PARAM_MASTER_GAIN,
    PARAM_MASTER_BYPASS,
    
    PARAM_COUNT
};

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    const clap_host_t* host;
#if VSTGUI_ENABLED
    MyPluginEditor* gui_editor;
#endif
    
    // EQ Parameters
    double param_values[PARAM_COUNT];
    
    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
