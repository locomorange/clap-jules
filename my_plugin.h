#pragma once

#include <clap/clap.h>
#include <complex>
#include <vector>
#include <atomic>
#include <mutex>

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

// Spectrum analyzer data structure
typedef struct {
    std::vector<float> magnitudes;  // Frequency magnitudes (0-1 range)
    std::vector<float> frequencies; // Corresponding frequencies in Hz
    std::atomic<bool> data_ready;
    std::mutex data_mutex;
    SpectrumDrawStyle draw_style;
    bool enabled;
} spectrum_data_t;

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
    bool spectrum_enabled;
    SpectrumDrawStyle spectrum_style;
} plugin_params_t;

// FFT and spectrum analysis data
typedef struct {
    std::vector<std::complex<float>> fft_buffer;
    std::vector<float> window_function;
    std::vector<float> input_buffer;
    size_t buffer_index;
    size_t frames_since_last_update;
    double sample_rate;
    spectrum_data_t spectrum_data;
} fft_data_t;

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    const clap_host_t* host;
    plugin_params_t params;
    fft_data_t fft_data;
#if VSTGUI_ENABLED
    MyPluginEditor* gui_editor;
#endif
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;

// Helper functions to access plugin data from GUI
#ifdef __cplusplus
extern "C" {
#endif

// Function to get spectrum data from plugin (thread-safe)
bool get_plugin_spectrum_data(const my_plugin_t* plugin, std::vector<float>& magnitudes, std::vector<float>& frequencies);

// Function to update spectrum parameters
void set_plugin_spectrum_enabled(my_plugin_t* plugin, bool enabled);
void set_plugin_spectrum_style(my_plugin_t* plugin, SpectrumDrawStyle style);

// FFT and spectrum analysis functions (for testing)
void init_fft_data(fft_data_t* fft_data, double sample_rate);
void cleanup_fft_data(fft_data_t* fft_data);
void generate_hann_window(std::vector<float>& window, size_t size);

#ifdef __cplusplus
}
#endif
