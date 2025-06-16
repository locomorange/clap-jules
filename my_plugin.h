#pragma once

#include <clap/clap.h>

#ifdef __cplusplus
// C++ includes only when compiling as C++
#include <complex>
#include <vector>
#include <atomic>
#include <mutex>
#endif

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
    PARAM_SPECTRUM_ENABLED,
    PARAM_SPECTRUM_STYLE,
    PARAM_COUNT
};

// Spectrum analyzer constants
#define SPECTRUM_FFT_SIZE 1024
#define SPECTRUM_HISTORY_SIZE 4
#define SPECTRUM_UPDATE_RATE 30  // Hz

// Spectrum drawing styles
enum SpectrumDrawStyle {
    SPECTRUM_STYLE_LINES = 0,
    SPECTRUM_STYLE_DOTS,
    SPECTRUM_STYLE_BINS,
    SPECTRUM_STYLE_FILLS,
    SPECTRUM_STYLE_COUNT
};

// Forward declaration of opaque C++ implementation
#ifdef __cplusplus
class SpectrumAnalyzer;
class FFTProcessor;
#else
typedef struct SpectrumAnalyzer SpectrumAnalyzer;
typedef struct FFTProcessor FFTProcessor;
#endif

// Plugin parameters structure (pure C)
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
    bool spectrum_enabled;
    SpectrumDrawStyle spectrum_style;
} plugin_params_t;

// Basic plugin structure (pure C with opaque pointers)
typedef struct {
    clap_plugin_t plugin;
    const clap_host_t* host;
    plugin_params_t params;
    FFTProcessor* fft_processor;       // Opaque C++ object
    SpectrumAnalyzer* spectrum_analyzer; // Opaque C++ object
#if VSTGUI_ENABLED
    MyPluginEditor* gui_editor;
#endif
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;

// C interface functions
#ifdef __cplusplus
extern "C" {
#endif

// FFT Processor C interface functions
FFTProcessor* fft_processor_create(double sample_rate);
void fft_processor_destroy(FFTProcessor* processor);
void fft_processor_process(FFTProcessor* processor, const float* input, size_t frame_count);
void fft_processor_get_spectrum_data(FFTProcessor* processor, float* magnitudes, float* frequencies, size_t* count);

// Spectrum Analyzer C interface functions  
SpectrumAnalyzer* spectrum_analyzer_create(void);
void spectrum_analyzer_destroy(SpectrumAnalyzer* analyzer);
void spectrum_analyzer_update_data(SpectrumAnalyzer* analyzer, const float* magnitudes, const float* frequencies, size_t count);
bool spectrum_analyzer_get_data(SpectrumAnalyzer* analyzer, float* magnitudes, float* frequencies, size_t* count);
void spectrum_analyzer_set_enabled(SpectrumAnalyzer* analyzer, bool enabled);
void spectrum_analyzer_set_style(SpectrumAnalyzer* analyzer, SpectrumDrawStyle style);

// Plugin helper functions
bool get_plugin_spectrum_data(const my_plugin_t* plugin, float* magnitudes, float* frequencies, size_t* count);
void set_plugin_spectrum_enabled(my_plugin_t* plugin, bool enabled);
void set_plugin_spectrum_style(my_plugin_t* plugin, SpectrumDrawStyle style);

#ifdef __cplusplus
}

// C++ interface functions (only available when compiling as C++)
#include <vector>

// Legacy C++ interface for backwards compatibility
bool get_plugin_spectrum_data(const my_plugin_t* plugin, std::vector<float>& magnitudes, std::vector<float>& frequencies);

// Testing functions (C++ only)
void init_fft_data(void* fft_data, double sample_rate);
void cleanup_fft_data(void* fft_data);
void generate_hann_window(std::vector<float>& window, size_t size);

#endif
