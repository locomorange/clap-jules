#pragma once

#include <clap/clap.h>
#include <cmath>

// Parameter IDs
#define PARAM_GAIN_ID 0

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    
    // Plugin state
    double gain_db;        // Gain in dB (-36 to +36)
    double gain_linear;    // Gain in linear scale (for processing)
    double sample_rate;    // Current sample rate
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
