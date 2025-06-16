// Windows compatibility - must be first
#ifdef _WIN32
#define _USE_MATH_DEFINES
#define NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

// Standard C++ includes
#include <cmath>
#include <algorithm>
#include <complex>
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc

// Ensure math constants are available on all platforms
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Plugin includes
#include "my_plugin.h"
#include <clap/plugin-features.h>

// GUI includes (platform-specific)
#if VSTGUI_ENABLED
#include "my_plugin_gui.h"
#include <clap/ext/gui.h>
#if defined(__linux__) && !defined(_WIN32)
#include "my_plugin_linux_extensions.h"
#endif
#endif

// --- Forward declarations of plugin functions ---
static bool my_plugin_init(const struct clap_plugin *plugin);
static void my_plugin_destroy(const struct clap_plugin *plugin);
static bool my_plugin_activate(const struct clap_plugin *plugin, double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count);
static void my_plugin_deactivate(const struct clap_plugin *plugin);
static bool my_plugin_start_processing(const struct clap_plugin *plugin);
static void my_plugin_stop_processing(const struct clap_plugin *plugin);
static void my_plugin_reset(const struct clap_plugin *plugin);
static clap_process_status my_plugin_process(const struct clap_plugin *plugin, const clap_process_t *process);
static const void *my_plugin_get_extension(const struct clap_plugin *plugin, const char *id);
static void my_plugin_on_main_thread(const struct clap_plugin *plugin);

// --- FFT and spectrum analysis functions ---
// C++ Implementation Classes (internal)
#ifdef __cplusplus

#include <vector>
#include <complex>
#include <atomic>
#include <mutex>

class FFTProcessor {
private:
    std::vector<std::complex<float>> fft_buffer;
    std::vector<float> window_function;
    std::vector<float> input_buffer;
    std::vector<float> magnitude_buffer;
    std::vector<float> frequency_buffer;
    size_t buffer_index;
    size_t frames_since_last_update;
    double sample_rate;
    std::mutex data_mutex;
    std::atomic<bool> data_ready;

    void perform_fft(std::vector<std::complex<float>>& data);
    void generate_hann_window(std::vector<float>& window, size_t size);
    void update_spectrum_data();

public:
    FFTProcessor(double sample_rate);
    ~FFTProcessor();
    void process(const float* input, size_t frame_count);
    bool get_spectrum_data(float* magnitudes, float* frequencies, size_t* count);
};

class SpectrumAnalyzer {
private:
    std::vector<float> current_magnitudes;
    std::vector<float> current_frequencies;
    std::mutex data_mutex;
    std::atomic<bool> enabled;
    SpectrumDrawStyle draw_style;

public:
    SpectrumAnalyzer();
    ~SpectrumAnalyzer();
    void update_data(const float* magnitudes, const float* frequencies, size_t count);
    bool get_data(float* magnitudes, float* frequencies, size_t* count);
    void set_enabled(bool enabled);
    void set_style(SpectrumDrawStyle style);
    bool is_enabled() const { return enabled.load(); }
    SpectrumDrawStyle get_style() const { return draw_style; }
};

#endif

// --- Plugin Descriptor ---
// Features array for the plugin descriptor  
static const char *const plugin_features[] = {CLAP_PLUGIN_FEATURE_AUDIO_EFFECT, nullptr};

static const clap_plugin_descriptor_t my_plugin_descriptor = {
    CLAP_VERSION,
    "com.example.soothe-pro", // id
    "Soothe Pro - Dynamic EQ", // name
    "Audio Innovations",       // vendor
    "https://example.com",     // url
    "https://example.com/manual", // manual_url
    "https://example.com/support", // support_url
    "1.0.0",                   // version
    "Professional dynamic EQ and resonance control plugin with Soothe2-style interface.", // description
    plugin_features, // features
};

//=============================================================================
// C++ Implementation Classes
//=============================================================================

#ifdef __cplusplus

// FFTProcessor Implementation
FFTProcessor::FFTProcessor(double sample_rate) 
    : buffer_index(0), frames_since_last_update(0), sample_rate(sample_rate), data_ready(false)
{
    // Initialize buffers
    fft_buffer.resize(SPECTRUM_FFT_SIZE);
    input_buffer.resize(SPECTRUM_FFT_SIZE);
    window_function.resize(SPECTRUM_FFT_SIZE);
    magnitude_buffer.resize(SPECTRUM_FFT_SIZE / 2);
    frequency_buffer.resize(SPECTRUM_FFT_SIZE / 2);
    
    // Generate Hann window
    generate_hann_window(window_function, SPECTRUM_FFT_SIZE);
    
    // Calculate frequency bins
    for (size_t i = 0; i < SPECTRUM_FFT_SIZE / 2; ++i) {
        frequency_buffer[i] = (float)i * (float)(sample_rate / 2) / (float)(SPECTRUM_FFT_SIZE / 2);
    }
}

FFTProcessor::~FFTProcessor() {
    // Destructor - vectors clean up automatically
}

void FFTProcessor::generate_hann_window(std::vector<float>& window, size_t size) {
    if (size <= 1) {
        if (size == 1) {
            window[0] = 1.0f;
        }
        return;
    }
    
    for (size_t i = 0; i < size; ++i) {
        double angle = 2.0 * M_PI * static_cast<double>(i) / static_cast<double>(size - 1);
        window[i] = static_cast<float>(0.5 * (1.0 - std::cos(angle)));
    }
}

void FFTProcessor::perform_fft(std::vector<std::complex<float>>& data) {
    const size_t N = data.size();
    if (N <= 1) return;
    
    // Bit-reversal permutation
    for (size_t i = 1, j = 0; i < N; ++i) {
        size_t bit = N >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }
    
    // Cooley-Tukey FFT
    for (size_t len = 2; len <= N; len <<= 1) {
        double angle = 2.0 * M_PI / static_cast<double>(len);
        std::complex<float> wlen(static_cast<float>(std::cos(angle)), static_cast<float>(std::sin(angle)));
        
        for (size_t i = 0; i < N; i += len) {
            std::complex<float> w(1.0f, 0.0f);
            for (size_t j = 0; j < len / 2; ++j) {
                std::complex<float> u = data[i + j];
                std::complex<float> v = data[i + j + len / 2] * w;
                data[i + j] = u + v;
                data[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

void FFTProcessor::process(const float* input, size_t frame_count) {
    for (size_t i = 0; i < frame_count; ++i) {
        input_buffer[buffer_index] = input[i];
        buffer_index = (buffer_index + 1) % SPECTRUM_FFT_SIZE;
        
        frames_since_last_update++;
        const size_t update_interval = (size_t)(sample_rate / SPECTRUM_UPDATE_RATE);
        
        if (frames_since_last_update >= update_interval && buffer_index == 0) {
            update_spectrum_data();
            frames_since_last_update = 0;
        }
    }
}

void FFTProcessor::update_spectrum_data() {
    // Copy input buffer and apply window function
    for (size_t i = 0; i < SPECTRUM_FFT_SIZE; ++i) {
        size_t idx = (buffer_index + i) % SPECTRUM_FFT_SIZE;
        fft_buffer[i] = std::complex<float>(input_buffer[idx] * window_function[i], 0.0f);
    }
    
    // Perform FFT
    perform_fft(fft_buffer);
    
    // Update magnitude data
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        const size_t num_bins = SPECTRUM_FFT_SIZE / 2;
        for (size_t i = 0; i < num_bins; ++i) {
            float magnitude = std::abs(fft_buffer[i]);
            float db = 20.0f * static_cast<float>(std::log10(std::max(magnitude, 1e-6f)));
            magnitude_buffer[i] = std::max(0.0f, std::min(1.0f, (db + 60.0f) / 60.0f));
        }
        data_ready.store(true);
    }
}

bool FFTProcessor::get_spectrum_data(float* magnitudes, float* frequencies, size_t* count) {
    std::lock_guard<std::mutex> lock(data_mutex);
    const size_t num_bins = SPECTRUM_FFT_SIZE / 2;
    *count = num_bins;
    
    // Always provide frequency data
    if (frequencies) {
        std::copy(frequency_buffer.begin(), frequency_buffer.end(), frequencies);
    }
    
    // Only provide magnitude data if FFT processing has occurred
    if (magnitudes) {
        if (data_ready.load()) {
            std::copy(magnitude_buffer.begin(), magnitude_buffer.end(), magnitudes);
        } else {
            // Initialize with zeros if no data ready yet
            std::fill(magnitudes, magnitudes + num_bins, 0.0f);
        }
    }
    
    return true;  // Always return true since we can provide frequency data
}

// SpectrumAnalyzer Implementation
SpectrumAnalyzer::SpectrumAnalyzer() : enabled(true), draw_style(SPECTRUM_STYLE_LINES) {
    current_magnitudes.resize(SPECTRUM_FFT_SIZE / 2, 0.0f);
    current_frequencies.resize(SPECTRUM_FFT_SIZE / 2, 0.0f);
}

SpectrumAnalyzer::~SpectrumAnalyzer() {
    // Destructor - vectors clean up automatically
}

void SpectrumAnalyzer::update_data(const float* magnitudes, const float* frequencies, size_t count) {
    std::lock_guard<std::mutex> lock(data_mutex);
    
    const size_t copy_count = std::min(count, current_magnitudes.size());
    if (magnitudes) {
        std::copy(magnitudes, magnitudes + copy_count, current_magnitudes.begin());
    }
    if (frequencies) {
        std::copy(frequencies, frequencies + copy_count, current_frequencies.begin());
    }
}

bool SpectrumAnalyzer::get_data(float* magnitudes, float* frequencies, size_t* count) {
    if (!enabled.load()) return false;
    
    std::lock_guard<std::mutex> lock(data_mutex);
    *count = current_magnitudes.size();
    
    if (magnitudes) {
        std::copy(current_magnitudes.begin(), current_magnitudes.end(), magnitudes);
    }
    if (frequencies) {
        std::copy(current_frequencies.begin(), current_frequencies.end(), frequencies);
    }
    
    return true;
}

void SpectrumAnalyzer::set_enabled(bool new_enabled) {
    enabled.store(new_enabled);
}

void SpectrumAnalyzer::set_style(SpectrumDrawStyle new_style) {
    draw_style = new_style;
}

#endif // __cplusplus

//=============================================================================
// C Interface Functions  
//=============================================================================

FFTProcessor* fft_processor_create(double sample_rate) {
#ifdef __cplusplus
    return new FFTProcessor(sample_rate);
#else
    return nullptr;
#endif
}

void fft_processor_destroy(FFTProcessor* processor) {
#ifdef __cplusplus
    delete processor;
#endif
}

void fft_processor_process(FFTProcessor* processor, const float* input, size_t frame_count) {
#ifdef __cplusplus
    if (processor) {
        processor->process(input, frame_count);
    }
#endif
}

void fft_processor_get_spectrum_data(FFTProcessor* processor, float* magnitudes, float* frequencies, size_t* count) {
#ifdef __cplusplus
    if (processor) {
        processor->get_spectrum_data(magnitudes, frequencies, count);
    } else {
        *count = 0;
    }
#else
    *count = 0;
#endif
}

SpectrumAnalyzer* spectrum_analyzer_create(void) {
#ifdef __cplusplus
    return new SpectrumAnalyzer();
#else
    return nullptr;
#endif
}

void spectrum_analyzer_destroy(SpectrumAnalyzer* analyzer) {
#ifdef __cplusplus
    delete analyzer;
#endif
}

void spectrum_analyzer_update_data(SpectrumAnalyzer* analyzer, const float* magnitudes, const float* frequencies, size_t count) {
#ifdef __cplusplus
    if (analyzer) {
        analyzer->update_data(magnitudes, frequencies, count);
    }
#endif
}

bool spectrum_analyzer_get_data(SpectrumAnalyzer* analyzer, float* magnitudes, float* frequencies, size_t* count) {
#ifdef __cplusplus
    if (analyzer) {
        return analyzer->get_data(magnitudes, frequencies, count);
    }
#endif
    *count = 0;
    return false;
}

void spectrum_analyzer_set_enabled(SpectrumAnalyzer* analyzer, bool enabled) {
#ifdef __cplusplus
    if (analyzer) {
        analyzer->set_enabled(enabled);
    }
#endif
}

void spectrum_analyzer_set_style(SpectrumAnalyzer* analyzer, SpectrumDrawStyle style) {
#ifdef __cplusplus
    if (analyzer) {
        analyzer->set_style(style);
    }
#endif
}

//=============================================================================
// Legacy compatibility functions (for testing)
//=============================================================================

void init_fft_data(void* fft_data, double sample_rate) {
    // Legacy function for testing compatibility
}

void cleanup_fft_data(void* fft_data) {
    // Legacy function for testing compatibility  
}

void generate_hann_window(std::vector<float>& window, size_t size) {
#ifdef __cplusplus
    if (size <= 1) {
        if (size == 1) {
            window[0] = 1.0f;
        }
        return;
    }
    
    for (size_t i = 0; i < size; ++i) {
        double angle = 2.0 * M_PI * static_cast<double>(i) / static_cast<double>(size - 1);
        window[i] = static_cast<float>(0.5 * (1.0 - std::cos(angle)));
    }
#endif
}

//=============================================================================
// Plugin Implementation
//=============================================================================
static bool my_plugin_init(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Initializing plugin\n");
    
    // Initialize parameters with default values
    self->params.cutoff = 1000.0;
    self->params.resonance = 1.0;
    self->params.drive = 0.0;
    self->params.output = 0.0;
    self->params.mix = 1.0;
    self->params.bypass = false;
    self->params.spectrum_enabled = true;
    self->params.spectrum_style = SPECTRUM_STYLE_LINES;
    
    for (int i = 0; i < 3; ++i) {
        self->params.eq_gain[i] = 0.0;
        self->params.eq_freq[i] = (i == 0) ? 200.0 : (i == 1) ? 1000.0 : 5000.0;
        self->params.eq_q[i] = 1.0;
    }
    
    // Initialize host pointer
    self->host = nullptr;
    
    // Initialize opaque pointers (will be created in activate)
    self->fft_processor = nullptr;
    self->spectrum_analyzer = nullptr;
    
    // Initialize GUI editor pointer
#if VSTGUI_ENABLED
    self->gui_editor = nullptr;
#endif
    
    return true;
}

static void my_plugin_destroy(const struct clap_plugin *plugin) {
    printf("MyPlugin: Destroying plugin\n");
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self) {
        // Cleanup FFT processor and spectrum analyzer
        if (self->fft_processor) {
            fft_processor_destroy(self->fft_processor);
            self->fft_processor = nullptr;
        }
        if (self->spectrum_analyzer) {
            spectrum_analyzer_destroy(self->spectrum_analyzer);
            self->spectrum_analyzer = nullptr;
        }
        
#if VSTGUI_ENABLED
        if (self->gui_editor) {
            delete self->gui_editor;
            self->gui_editor = nullptr;
        }
#endif
        free(self);
    }
}

static bool my_plugin_activate(const struct clap_plugin *plugin, double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count) {
    printf("MyPlugin: Activating plugin (Sample Rate: %.2f, Min Frames: %u, Max Frames: %u)\n", sample_rate, min_frames_count, max_frames_count);
    
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    // Create FFT processor and spectrum analyzer
    self->fft_processor = fft_processor_create(sample_rate);
    self->spectrum_analyzer = spectrum_analyzer_create();
    
    if (!self->fft_processor || !self->spectrum_analyzer) {
        printf("MyPlugin: Failed to create FFT processor or spectrum analyzer\n");
        return false;
    }
    
    printf("MyPlugin: FFT processor and spectrum analyzer initialized\n");
    return true;
}

static void my_plugin_deactivate(const struct clap_plugin *plugin) {
    printf("MyPlugin: Deactivating plugin\n");
    // Free resources allocated in activate
}

static bool my_plugin_start_processing(const struct clap_plugin *plugin) {
    printf("MyPlugin: Starting processing\n");
    return true;
}

static void my_plugin_stop_processing(const struct clap_plugin *plugin) {
    printf("MyPlugin: Stopping processing\n");
}

static void my_plugin_reset(const struct clap_plugin *plugin) {
    printf("MyPlugin: Resetting plugin\n");
    // Reset plugin state (e.g., clear buffers, reset parameters)
}

static clap_process_status my_plugin_process(const struct clap_plugin *plugin, const clap_process_t *process) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    // Example: Process audio from input to output (stereo) with spectrum analysis
    if (process->audio_outputs_count > 0 && process->audio_inputs_count > 0) {
        clap_audio_buffer_t *out_buf = &process->audio_outputs[0];
        const clap_audio_buffer_t *in_buf = &process->audio_inputs[0];
        
        if (out_buf->channel_count >= 2 && in_buf->channel_count >= 2 && out_buf->data32 && in_buf->data32) {
            // Process audio and perform spectrum analysis on left channel
            for (uint32_t i = 0; i < process->frames_count; ++i) {
                out_buf->data32[0][i] = in_buf->data32[0][i]; // Left channel
                out_buf->data32[1][i] = in_buf->data32[1][i]; // Right channel
            }
            
            // Perform spectrum analysis on the left channel using new FFT processor
            if (self->params.spectrum_enabled && self->fft_processor) {
                fft_processor_process(self->fft_processor, in_buf->data32[0], process->frames_count);
                
                // Update spectrum analyzer with new data if available
                if (self->spectrum_analyzer) {
                    float magnitudes[SPECTRUM_FFT_SIZE / 2];
                    float frequencies[SPECTRUM_FFT_SIZE / 2];
                    size_t count = 0;
                    
                    fft_processor_get_spectrum_data(self->fft_processor, magnitudes, frequencies, &count);
                    if (count > 0) {
                        spectrum_analyzer_update_data(self->spectrum_analyzer, magnitudes, frequencies, count);
                    }
                }
            }
        }
    }
    return CLAP_PROCESS_CONTINUE;
}

// --- GUI Extension Implementation ---
#if VSTGUI_ENABLED
static bool my_plugin_gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->isApiSupported(api, is_floating);
    }
    return false;
}

static bool my_plugin_gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->getPreferredApi(api, is_floating);
    }
    return false;
}

static bool my_plugin_gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->create(api, is_floating);
    }
    return false;
}

static void my_plugin_gui_destroy(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        self->gui_editor->destroy();
    }
}

static bool my_plugin_gui_set_scale(const clap_plugin_t *plugin, double scale) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->setScale(scale);
    }
    return false;
}

static bool my_plugin_gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->getSize(width, height);
    }
    return false;
}

static bool my_plugin_gui_can_resize(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->canResize();
    }
    return false;
}

static bool my_plugin_gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->getResizeHints(hints);
    }
    return false;
}

static bool my_plugin_gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->adjustSize(width, height);
    }
    return false;
}

static bool my_plugin_gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->setSize(width, height);
    }
    return false;
}

static bool my_plugin_gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->setParent(window);
    }
    return false;
}

static bool my_plugin_gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->setTransient(window);
    }
    return false;
}

static void my_plugin_gui_suggest_title(const clap_plugin_t *plugin, const char *title) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        self->gui_editor->suggestTitle(title);
    }
}

static bool my_plugin_gui_show(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->show();
    }
    return false;
}

static bool my_plugin_gui_hide(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->hide();
    }
    return false;
}

static const clap_plugin_gui_t my_gui_extension = {
    my_plugin_gui_is_api_supported,
    my_plugin_gui_get_preferred_api,
    my_plugin_gui_create,
    my_plugin_gui_destroy,
    my_plugin_gui_set_scale,
    my_plugin_gui_get_size,
    my_plugin_gui_can_resize,
    my_plugin_gui_get_resize_hints,
    my_plugin_gui_adjust_size,
    my_plugin_gui_set_size,
    my_plugin_gui_set_parent,
    my_plugin_gui_set_transient,
    my_plugin_gui_suggest_title,
    my_plugin_gui_show,
    my_plugin_gui_hide
};
#endif // VSTGUI_ENABLED



static const void *my_plugin_get_extension(const struct clap_plugin *plugin, const char *id) {
    // Example: if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) return &my_audio_ports_extension;
    // Example: if (strcmp(id, CLAP_EXT_PARAMS) == 0) return &my_params_extension;
    printf("MyPlugin: Host requesting extension: %s\n", id);
    
#if VSTGUI_ENABLED
    if (strcmp(id, CLAP_EXT_GUI) == 0) {
        return &my_gui_extension;
    }
#if defined(__linux__)
    if (strcmp(id, CLAP_EXT_TIMER_SUPPORT) == 0) {
        return &my_timer_support_extension;
    }
    if (strcmp(id, CLAP_EXT_POSIX_FD_SUPPORT) == 0) {
        return &my_posix_fd_support_extension;
    }
#endif
#endif
    
    return NULL; // No other extensions supported in this basic example
}

static void my_plugin_on_main_thread(const struct clap_plugin *plugin) {
    // Called by the host to perform tasks that must run on the main thread.
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    // Update spectrum display if GUI is available
#if VSTGUI_ENABLED
    if (self && self->gui_editor) {
        self->gui_editor->updateSpectrumDisplay();
    }
#endif
}

// --- Plugin Entry Point (clap_plugin_entry) ---
// This is not directly part of the clap_plugin_t struct but is essential.
// It's usually defined in the factory.

// --- Plugin Factory ---
// This structure is responsible for creating plugin instances.

static uint32_t my_factory_get_plugin_count(const struct clap_plugin_factory *factory) {
    return 1; // We have one plugin in this factory
}

static const clap_plugin_descriptor_t *my_factory_get_plugin_descriptor(const struct clap_plugin_factory *factory, uint32_t index) {
    if (index == 0) {
        return &my_plugin_descriptor;
    }
    return NULL;
}

static const clap_plugin_t *my_factory_create_plugin(const struct clap_plugin_factory *factory, const clap_host_t *host, const char *plugin_id) {
    if (strcmp(plugin_id, my_plugin_descriptor.id) != 0) {
        fprintf(stderr, "MyPlugin: Error - incorrect plugin ID requested: %s\n", plugin_id);
        return NULL;
    }

    my_plugin_t *self = (my_plugin_t *)calloc(1, sizeof(my_plugin_t));
    if (!self) {
        fprintf(stderr, "MyPlugin: Error - failed to allocate memory for plugin instance\n");
        return NULL;
    }

    // Store host pointer for extension access
    self->host = host;

    // Initialize GUI editor
#if VSTGUI_ENABLED
    self->gui_editor = new MyPluginEditor(host, self);
    if (!self->gui_editor) {
        fprintf(stderr, "MyPlugin: Error - failed to create GUI editor\n");
        free(self);
        return NULL;
    }
#endif

    self->plugin.desc = &my_plugin_descriptor;
    self->plugin.plugin_data = self; // Point to ourself for context
    self->plugin.init = my_plugin_init;
    self->plugin.destroy = my_plugin_destroy;
    self->plugin.activate = my_plugin_activate;
    self->plugin.deactivate = my_plugin_deactivate;
    self->plugin.start_processing = my_plugin_start_processing;
    self->plugin.stop_processing = my_plugin_stop_processing;
    self->plugin.reset = my_plugin_reset;
    self->plugin.process = my_plugin_process;
    self->plugin.get_extension = my_plugin_get_extension;
    self->plugin.on_main_thread = my_plugin_on_main_thread;

    printf("MyPlugin: Plugin instance created successfully.\n");
    return &self->plugin;
}

const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory = {
    my_factory_get_plugin_count,
    my_factory_get_plugin_descriptor,
    my_factory_create_plugin,
};

//=============================================================================
// Helper Functions for GUI Access
//=============================================================================

// C interface helper functions
bool get_plugin_spectrum_data(const my_plugin_t* plugin, float* magnitudes, float* frequencies, size_t* count) {
    if (!plugin || !plugin->spectrum_analyzer) {
        *count = 0;
        return false;
    }
    
    return spectrum_analyzer_get_data(plugin->spectrum_analyzer, magnitudes, frequencies, count);
}

void set_plugin_spectrum_enabled(my_plugin_t* plugin, bool enabled) {
    if (plugin) {
        plugin->params.spectrum_enabled = enabled;
        if (plugin->spectrum_analyzer) {
            spectrum_analyzer_set_enabled(plugin->spectrum_analyzer, enabled);
        }
    }
}

void set_plugin_spectrum_style(my_plugin_t* plugin, SpectrumDrawStyle style) {
    if (plugin) {
        plugin->params.spectrum_style = style;
        if (plugin->spectrum_analyzer) {
            spectrum_analyzer_set_style(plugin->spectrum_analyzer, style);
        }
    }
}

// C++ interface helper functions (for backwards compatibility)
#ifdef __cplusplus
bool get_plugin_spectrum_data(const my_plugin_t* plugin, std::vector<float>& magnitudes, std::vector<float>& frequencies) {
    if (!plugin || !plugin->spectrum_analyzer) {
        return false;
    }
    
    float temp_magnitudes[SPECTRUM_FFT_SIZE / 2];
    float temp_frequencies[SPECTRUM_FFT_SIZE / 2];
    size_t count = 0;
    
    bool success = spectrum_analyzer_get_data(plugin->spectrum_analyzer, temp_magnitudes, temp_frequencies, &count);
    if (success && count > 0) {
        magnitudes.resize(count);
        frequencies.resize(count);
        std::copy(temp_magnitudes, temp_magnitudes + count, magnitudes.begin());
        std::copy(temp_frequencies, temp_frequencies + count, frequencies.begin());
        return true;
    }
    
    return false;
}
#endif

//=============================================================================
// CLAP Entry Point
//=============================================================================

// --- CLAP Entry Point Functions ---
// Convert lambdas to regular functions to improve Windows MSVC compatibility
static bool clap_entry_init(const char *plugin_path) {
    printf("MyPlugin: clap_entry.init called (path: %s)\n", plugin_path);
    // Perform any global library initialization here if needed
    return true;
}

static void clap_entry_deinit() {
    printf("MyPlugin: clap_entry.deinit called\n");
    // Perform any global library cleanup here if needed
}

static const void* clap_entry_get_factory(const char *factory_id) {
    printf("MyPlugin: clap_entry.get_factory called (ID: %s)\n", factory_id);
    if (strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID) == 0) {
        return &my_plugin_factory;
    }
    // To support other factory types, check their specific IDs here.
    // For example, CLAP_PLUGIN_VOICE_INFO_FACTORY_ID for voice info.
    // Or CLAP_PLUGIN_REMOTABLE_CONTROLS_FACTORY_ID for remotable controls.
    fprintf(stderr, "MyPlugin: Unknown factory ID requested: %s\n", factory_id);
    return NULL;
}

// --- CLAP Entry Point ---
// This is the main entry point that the host will look for.
CLAP_EXPORT const clap_plugin_entry_t clap_entry = {
    CLAP_VERSION,
    clap_entry_init,
    clap_entry_deinit,
    clap_entry_get_factory
};
