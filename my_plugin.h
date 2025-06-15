#pragma once

#include <clap/clap.h>
#include <clap/ext/params.h>
#include <clap/ext/gui.h>
#include <complex>
#include <vector>
#include <memory>

// Forward declarations
struct SpectrumAnalyzer;

// Parameter IDs
enum {
    PARAM_SPECTRUM_DRAWING_STYLE = 0,
    PARAM_COUNT
};

// Drawing styles for spectrum analyzer
enum SpectrumDrawingStyle {
    STYLE_LINES = 0,
    STYLE_DOTS = 1,
    STYLE_BINS = 2,
    STYLE_FILLS = 3,
    STYLE_COUNT
};

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    
    // Audio processing
    double sample_rate;
    uint32_t max_frames_count;
    
    // Spectrum analyzer
    std::unique_ptr<SpectrumAnalyzer> spectrum_analyzer;
    
    // Parameters
    float spectrum_drawing_style;
    
    // GUI (will be implemented later)
    void* gui_context;
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
