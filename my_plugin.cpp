// Define math constants for Windows compatibility - must be before all includes
#ifdef _WIN32
#define _USE_MATH_DEFINES
#define NOMINMAX
#endif

#include "my_plugin.h"
#if VSTGUI_ENABLED
#include "my_plugin_gui.h"
#include <clap/ext/gui.h>
#include "my_plugin_linux_extensions.h"
#endif
#include <clap/plugin-features.h>
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <cmath>    // For FFT calculations
#include <algorithm>
#include <complex>

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
void init_fft_data(fft_data_t* fft_data, double sample_rate);
void cleanup_fft_data(fft_data_t* fft_data);
void generate_hann_window(std::vector<float>& window, size_t size);
static void perform_fft(std::vector<std::complex<float>>& data);
static void process_spectrum_analysis(my_plugin_t* self, const float* audio_data, uint32_t frame_count);
static void update_spectrum_data(fft_data_t* fft_data);

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
// FFT and Spectrum Analysis Implementation
//=============================================================================

void init_fft_data(fft_data_t* fft_data, double sample_rate) {
    fft_data->sample_rate = sample_rate;
    fft_data->buffer_index = 0;
    fft_data->frames_since_last_update = 0;
    
    // Initialize buffers
    fft_data->fft_buffer.resize(SPECTRUM_FFT_SIZE);
    fft_data->input_buffer.resize(SPECTRUM_FFT_SIZE);
    fft_data->window_function.resize(SPECTRUM_FFT_SIZE);
    
    // Generate Hann window for better frequency resolution
    generate_hann_window(fft_data->window_function, SPECTRUM_FFT_SIZE);
    
    // Initialize spectrum data
    const size_t num_bins = SPECTRUM_FFT_SIZE / 2; // Only use positive frequencies
    fft_data->spectrum_data.magnitudes.resize(num_bins);
    fft_data->spectrum_data.frequencies.resize(num_bins);
    
    // Calculate frequency bins (logarithmic scale from 20Hz to 20kHz)
    for (size_t i = 0; i < num_bins; ++i) {
        float freq = (float)i * (float)(sample_rate / 2) / (float)num_bins;
        fft_data->spectrum_data.frequencies[i] = freq;
        fft_data->spectrum_data.magnitudes[i] = 0.0f;
    }
    
    fft_data->spectrum_data.data_ready.store(false);
    fft_data->spectrum_data.draw_style = SPECTRUM_STYLE_LINES;
    fft_data->spectrum_data.enabled = true;
}

void cleanup_fft_data(fft_data_t* fft_data) {
    std::lock_guard<std::mutex> lock(fft_data->spectrum_data.data_mutex);
    fft_data->fft_buffer.clear();
    fft_data->input_buffer.clear();
    fft_data->window_function.clear();
    fft_data->spectrum_data.magnitudes.clear();
    fft_data->spectrum_data.frequencies.clear();
}

void generate_hann_window(std::vector<float>& window, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        window[i] = 0.5f * (1.0f - cosf(2.0f * M_PI * i / (size - 1)));
    }
}

// Simple Cooley-Tukey FFT implementation
static void perform_fft(std::vector<std::complex<float>>& data) {
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
        float angle = 2.0f * M_PI / len;
        std::complex<float> wlen(cosf(angle), sinf(angle));
        
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

static void process_spectrum_analysis(my_plugin_t* self, const float* audio_data, uint32_t frame_count) {
    if (!self->fft_data.spectrum_data.enabled) {
        return;
    }
    
    for (uint32_t i = 0; i < frame_count; ++i) {
        // Fill input buffer
        self->fft_data.input_buffer[self->fft_data.buffer_index] = audio_data[i];
        self->fft_data.buffer_index = (self->fft_data.buffer_index + 1) % SPECTRUM_FFT_SIZE;
        
        // Check if it's time to update spectrum
        self->fft_data.frames_since_last_update++;
        const size_t update_interval = (size_t)(self->fft_data.sample_rate / SPECTRUM_UPDATE_RATE);
        
        if (self->fft_data.frames_since_last_update >= update_interval && self->fft_data.buffer_index == 0) {
            update_spectrum_data(&self->fft_data);
            self->fft_data.frames_since_last_update = 0;
        }
    }
}

static void update_spectrum_data(fft_data_t* fft_data) {
    // Copy input buffer and apply window function
    for (size_t i = 0; i < SPECTRUM_FFT_SIZE; ++i) {
        size_t idx = (fft_data->buffer_index + i) % SPECTRUM_FFT_SIZE;
        fft_data->fft_buffer[i] = std::complex<float>(
            fft_data->input_buffer[idx] * fft_data->window_function[i], 0.0f);
    }
    
    // Perform FFT
    perform_fft(fft_data->fft_buffer);
    
    // Update spectrum data with thread safety
    {
        std::lock_guard<std::mutex> lock(fft_data->spectrum_data.data_mutex);
        
        const size_t num_bins = SPECTRUM_FFT_SIZE / 2;
        for (size_t i = 0; i < num_bins; ++i) {
            float magnitude = std::abs(fft_data->fft_buffer[i]);
            // Convert to dB scale and normalize
            float db = 20.0f * log10f(std::max(magnitude, 1e-6f));
            // Normalize to 0-1 range (assuming -60dB to 0dB range)
            fft_data->spectrum_data.magnitudes[i] = std::max(0.0f, std::min(1.0f, (db + 60.0f) / 60.0f));
        }
        
        fft_data->spectrum_data.data_ready.store(true);
    }
}

//=============================================================================
// Plugin Implementation
//=============================================================================


// --- Plugin Implementation ---
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
    
    return true;
}

static void my_plugin_destroy(const struct clap_plugin *plugin) {
    printf("MyPlugin: Destroying plugin\n");
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self) {
        // Cleanup FFT data
        cleanup_fft_data(&self->fft_data);
        
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
    
    // Initialize FFT data for spectrum analysis
    init_fft_data(&self->fft_data, sample_rate);
    
    printf("MyPlugin: FFT data initialized for spectrum analysis\n");
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
            
            // Perform spectrum analysis on the left channel
            if (self->params.spectrum_enabled) {
                process_spectrum_analysis(self, in_buf->data32[0], process->frames_count);
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

bool get_plugin_spectrum_data(const my_plugin_t* plugin, std::vector<float>& magnitudes, std::vector<float>& frequencies) {
    if (!plugin || !plugin->fft_data.spectrum_data.data_ready.load()) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(plugin->fft_data.spectrum_data.data_mutex));
    
    if (plugin->fft_data.spectrum_data.magnitudes.size() != magnitudes.size() ||
        plugin->fft_data.spectrum_data.frequencies.size() != frequencies.size()) {
        magnitudes.resize(plugin->fft_data.spectrum_data.magnitudes.size());
        frequencies.resize(plugin->fft_data.spectrum_data.frequencies.size());
    }
    
    magnitudes = plugin->fft_data.spectrum_data.magnitudes;
    frequencies = plugin->fft_data.spectrum_data.frequencies;
    
    return true;
}

void set_plugin_spectrum_enabled(my_plugin_t* plugin, bool enabled) {
    if (plugin) {
        plugin->params.spectrum_enabled = enabled;
        plugin->fft_data.spectrum_data.enabled = enabled;
    }
}

void set_plugin_spectrum_style(my_plugin_t* plugin, SpectrumDrawStyle style) {
    if (plugin) {
        plugin->params.spectrum_style = style;
        plugin->fft_data.spectrum_data.draw_style = style;
    }
}

//=============================================================================
// CLAP Entry Point
//=============================================================================

// --- CLAP Entry Point ---
// This is the main entry point that the host will look for.
CLAP_EXPORT const clap_plugin_entry_t clap_entry = {
    CLAP_VERSION,
    // init: Called once when the library is loaded.
    [](const char *plugin_path) -> bool {
        printf("MyPlugin: clap_entry.init called (path: %s)\n", plugin_path);
        // Perform any global library initialization here if needed
        return true;
    },
    // deinit: Called once when the library is unloaded.
    []() -> void {
        printf("MyPlugin: clap_entry.deinit called\n");
        // Perform any global library cleanup here if needed
    },
    // get_factory: Returns a factory based on its ID.
    [](const char *factory_id) -> const void * {
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
};
