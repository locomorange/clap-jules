#pragma once

#include <clap/clap.h>

// Forward declaration
#if VSTGUI_ENABLED
class MyPluginEditor;
#endif

// Parameter IDs
enum PluginParamIds {
    PARAM_CUTOFF = 0,
    PARAM_RESONANCE,
    PARAM_DRIVE,
    PARAM_OUTPUT,
    PARAM_MIX,
    PARAM_BYPASS,
    PARAM_EQ_GAIN1,
    PARAM_EQ_FREQ1,
    PARAM_EQ_Q1,
    PARAM_EQ_GAIN2,
    PARAM_EQ_FREQ2,
    PARAM_EQ_Q2,
    PARAM_EQ_GAIN3,
    PARAM_EQ_FREQ3,
    PARAM_EQ_Q3,
    PARAM_COUNT
};

// Plugin parameters structure
typedef struct {
    double cutoff;
    double resonance;
    double drive;
    double output;
    double mix;
    bool bypass;
    double eq_gain[3];
    double eq_freq[3];
    double eq_q[3];
} plugin_params_t;

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    const clap_host_t* host;
    plugin_params_t params;
#if VSTGUI_ENABLED
    MyPluginEditor* gui_editor;
#endif
    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
