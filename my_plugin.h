#pragma once

#include <clap/clap.h>
#include <clap/ext/params.h>
#include <clap/ext/gui.h>
#include <complex>
#include <vector>
#include <memory>
#include <cstring> // For memset

// Forward declarations
struct SpectrumAnalyzer;
class SpectrumGUI;

#if VSTGUI_ENABLED
class MyPluginEditor;
#endif

// Parameter IDs - Combined spectrum analyzer and EQ parameters
enum PluginParamIds {
    // Spectrum analyzer parameters
    PARAM_SPECTRUM_DRAWING_STYLE = 0,
    
    // EQ parameters  
    PARAM_CUTOFF,

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


// Drawing styles for spectrum analyzer
enum SpectrumDrawingStyle {
    STYLE_LINES = 0,
    STYLE_DOTS = 1,
    STYLE_BINS = 2,
    STYLE_FILLS = 3,
    STYLE_COUNT
};

// Plugin parameters structure - Combined parameters
typedef struct {
    // Spectrum analyzer parameters
    float spectrum_drawing_style;
    
    // EQ parameters

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


// Basic plugin structure - Combined functionality
typedef struct my_plugin_t {
    clap_plugin_t plugin;
    const clap_host_t* host;
    plugin_params_t params;
    
    // Audio processing
    double sample_rate;
    uint32_t max_frames_count;
    
    // Spectrum analyzer
    std::unique_ptr<SpectrumAnalyzer> spectrum_analyzer;
    
    // GUI
    std::unique_ptr<SpectrumGUI> gui;

#if VSTGUI_ENABLED
    MyPluginEditor* gui_editor;
#endif

    // Constructor to initialize the struct properly
    my_plugin_t() : host(nullptr), sample_rate(44100.0), max_frames_count(512)
#if VSTGUI_ENABLED
        , gui_editor(nullptr)
#endif
    {
        // Initialize plugin struct to zero
        memset(&plugin, 0, sizeof(plugin));
        memset(&params, 0, sizeof(params));
    }
    
    // Destructor declaration (implementation in .cpp file)
    ~my_plugin_t();
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
