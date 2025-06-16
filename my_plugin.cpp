#include "my_plugin.h"
#include "graphics/skia_graphics.h"
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc
#include <cmath>    // For sin/cos
#include <memory>   // For std::make_unique, std::unique_ptr
#include <algorithm> // For std::fill, std::max
#include <complex>  // For std::complex
#include <clap/ext/gui.h>

// Simple FFT implementation (Cooley-Tukey algorithm)
namespace {
    void fft(std::vector<std::complex<float>>& x) {
        const size_t N = x.size();
        if (N <= 1) return;
        
        // Divide
        std::vector<std::complex<float>> even(N/2), odd(N/2);
        for (size_t i = 0; i < N/2; ++i) {
            even[i] = x[i*2];
            odd[i] = x[i*2+1];
        }
        
        // Conquer
        fft(even);
        fft(odd);
        
        // Combine
        for (size_t i = 0; i < N/2; ++i) {
            std::complex<float> t = std::polar(1.0f, -2.0f * static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(N)) * odd[i];
            x[i] = even[i] + t;
            x[i+N/2] = even[i] - t;
        }
    }
    
    void process_spectrum(my_plugin_t* self, const float* audio_data, uint32_t frame_count) {
        if (!self || frame_count == 0) return;
        
        // Add incoming audio to input buffer
        for (uint32_t i = 0; i < frame_count; ++i) {
            self->fft_input_buffer[self->input_buffer_pos] = audio_data[i];
            self->input_buffer_pos = (self->input_buffer_pos + 1) % FFT_SIZE;
            
            // When buffer is full, process FFT
            if (self->input_buffer_pos == 0) {
                // Copy input buffer to FFT buffer
                for (size_t j = 0; j < FFT_SIZE; ++j) {
                    self->fft_buffer[j] = std::complex<float>(self->fft_input_buffer[j], 0.0f);
                }
                
                // Apply window function (Hann window)
                for (size_t j = 0; j < FFT_SIZE; ++j) {
                    float window = 0.5f * (1.0f - cos(2.0f * static_cast<float>(M_PI) * static_cast<float>(j) / static_cast<float>(FFT_SIZE - 1)));
                    self->fft_buffer[j] *= window;
                }
                
                // Perform FFT
                fft(self->fft_buffer);
                
                // Calculate magnitudes and smooth
                for (size_t j = 0; j < SPECTRUM_BINS; ++j) {
                    float magnitude = std::abs(self->fft_buffer[j]);
                    // Convert to dB scale
                    magnitude = 20.0f * log10f(std::max(magnitude, 1e-8f));
                    // Normalize to 0-1 range (assuming input range of -80dB to 0dB)
                    magnitude = std::max(0.0f, std::min(1.0f, (magnitude + 80.0f) / 80.0f));
                    
                    // Apply smoothing
                    self->smoothed_spectrum[j] = self->spectrum_smoothing * self->smoothed_spectrum[j] + 
                                               (1.0f - self->spectrum_smoothing) * magnitude;
                    self->spectrum_magnitudes[j] = magnitude;
                }
            }
        }
    }
}

// --- Forward declarations of plugin functions ---
CLAP_EXPORT bool my_plugin_init(const struct clap_plugin *plugin);
CLAP_EXPORT void my_plugin_destroy(const struct clap_plugin *plugin);
CLAP_EXPORT bool my_plugin_activate(const struct clap_plugin *plugin, double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count);
static void my_plugin_deactivate(const struct clap_plugin *plugin);
static bool my_plugin_start_processing(const struct clap_plugin *plugin);
static void my_plugin_stop_processing(const struct clap_plugin *plugin);
static void my_plugin_reset(const struct clap_plugin *plugin);
CLAP_EXPORT clap_process_status my_plugin_process(const struct clap_plugin *plugin, const clap_process_t *process);
static const void *my_plugin_get_extension(const struct clap_plugin *plugin, const char *id);
static void my_plugin_on_main_thread(const struct clap_plugin *plugin);

// --- Plugin-specific rendering functions ---
static void my_plugin_render_content(my_plugin_t *self);
static bool my_plugin_present_graphics(my_plugin_t *self);

// --- GUI Extension Function Declarations ---
static bool my_plugin_gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating);
static bool my_plugin_gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating);
static bool my_plugin_gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating);
static void my_plugin_gui_destroy(const clap_plugin_t *plugin);
static bool my_plugin_gui_set_scale(const clap_plugin_t *plugin, double scale);
static bool my_plugin_gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height);
static bool my_plugin_gui_can_resize(const clap_plugin_t *plugin);
static bool my_plugin_gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints);
static bool my_plugin_gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height);
static bool my_plugin_gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height);
static bool my_plugin_gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window);
static bool my_plugin_gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window);
static void my_plugin_gui_suggest_title(const clap_plugin_t *plugin, const char *title);
static bool my_plugin_gui_show(const clap_plugin_t *plugin);
static bool my_plugin_gui_hide(const clap_plugin_t *plugin);

// --- GUI Extension Implementation ---
static const clap_plugin_gui_t my_plugin_gui = {
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
    my_plugin_gui_hide,
};

// --- Plugin Descriptor ---
// Features array for the plugin descriptor
static const char *const plugin_features[] = {"audio-effect", "analyzer", nullptr};

static const clap_plugin_descriptor_t my_plugin_descriptor = {
    CLAP_VERSION,
    "com.example.myplugin", // id
    "FFT Spectrum Analyzer", // name
    "My Company",           // vendor
    "https://example.com",  // url
    "https://example.com/bugtracker", // manual_url
    "https://example.com/support",    // support_url
    "0.1.0",                // version
    "Real-time FFT spectrum analyzer with 4 drawing modes (lines, dots, bins, fill).", // description
    plugin_features, // features
};


// --- Plugin Implementation ---
CLAP_EXPORT bool my_plugin_init(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Initializing plugin\n");
    
    // Initialize GUI state
    self->gui_created = false;
    self->gui_visible = false;
    self->gui_width = 640;  // Wider for spectrum display
    self->gui_height = 480; // Taller for spectrum display
    self->gui_api = nullptr;
    self->gui_is_floating = false;
    self->native_window = nullptr;
    self->needs_redraw = true;
#if defined(__linux__) && defined(HAVE_X11)
    self->x11_renderer = nullptr;
#endif
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    self->win32_renderer = nullptr;
#endif
    
    // Initialize FFT and spectrum analyzer buffers
    self->fft_input_buffer.resize(FFT_SIZE, 0.0f);
    self->fft_buffer.resize(FFT_SIZE);
    self->spectrum_magnitudes.resize(SPECTRUM_BINS, 0.0f);
    self->smoothed_spectrum.resize(SPECTRUM_BINS, 0.0f);
    self->input_buffer_pos = 0;
    self->draw_mode = SpectrumDrawMode::LINES;
    self->spectrum_smoothing = 0.7f;
    self->sample_rate = 44100.0;
    
    // Initialize graphics system and demonstrate basic usage
    printf("MyPlugin: Graphics backend - %s\n", clap_jules::graphics::getGraphicsBackendInfo().c_str());
    printf("MyPlugin: Skia available - %s\n", clap_jules::graphics::isSkiaAvailable() ? "Yes" : "No");
    
    // Initialize your plugin state here
    return true;
}

CLAP_EXPORT void my_plugin_destroy(const struct clap_plugin *plugin) {
    printf("MyPlugin: Destroying plugin\n");
    // Free any resources allocated in init
}

CLAP_EXPORT bool my_plugin_activate(const struct clap_plugin *plugin, double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Activating plugin (Sample Rate: %.2f, Min Frames: %u, Max Frames: %u)\n", sample_rate, min_frames_count, max_frames_count);
    
    // Store sample rate for spectrum analysis
    self->sample_rate = sample_rate;
    
    // Allocate and prepare resources needed for processing (e.g., buffers)
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

CLAP_EXPORT clap_process_status my_plugin_process(const struct clap_plugin *plugin, const clap_process_t *process) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    // Process audio from input to output (pass-through)
    if (process->audio_outputs_count > 0 && process->audio_inputs_count > 0) {
        clap_audio_buffer_t *out_buf = &process->audio_outputs[0];
        const clap_audio_buffer_t *in_buf = &process->audio_inputs[0];

        if (out_buf->channel_count >= 1 && in_buf->channel_count >= 1 && 
            out_buf->data32 && in_buf->data32 && process->frames_count > 0) {
            
            // Copy input to output (pass-through)
            for (uint32_t ch = 0; ch < std::min(out_buf->channel_count, in_buf->channel_count); ++ch) {
                for (uint32_t i = 0; i < process->frames_count; ++i) {
                    out_buf->data32[ch][i] = in_buf->data32[ch][i];
                }
            }
            
            // Process spectrum analysis on the first channel (left or mono)
            process_spectrum(self, in_buf->data32[0], process->frames_count);
        }
    }
    
    return CLAP_PROCESS_CONTINUE;
}

static const void *my_plugin_get_extension(const struct clap_plugin *plugin, const char *id) {
    printf("MyPlugin: Host requesting extension: %s\n", id);
    
    if (strcmp(id, CLAP_EXT_GUI) == 0) {
        printf("MyPlugin: Returning GUI extension\n");
        return &my_plugin_gui;
    }
    
    // Example: if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) return &my_audio_ports_extension;
    // Example: if (strcmp(id, CLAP_EXT_PARAMS) == 0) return &my_params_extension;
    return NULL; // Extension not supported
}

static void my_plugin_on_main_thread(const struct clap_plugin *plugin) {
    // Called by the host to perform tasks that must run on the main thread.
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    // Cycle through drawing modes for demo purposes (every 5 seconds)
    static int mode_counter = 0;
    mode_counter++;
    if (mode_counter % 300 == 0) { // Assuming 60 FPS, 300 frames = 5 seconds
        self->draw_mode = (SpectrumDrawMode)(((int)self->draw_mode + 1) % 4);
        printf("MyPlugin: Switched to drawing mode %d\n", (int)self->draw_mode);
    }
    
    // If GUI is visible, update the rendering
    if (self->gui_created && self->gui_visible && self->graphics_context) {
        my_plugin_render_content(self);
        my_plugin_present_graphics(self);
    }
}

// --- Plugin-specific rendering functions ---

// Spectrum drawing functions
namespace {
    void draw_spectrum_lines(my_plugin_t *self, float spectrum_x, float spectrum_y, float spectrum_width, float spectrum_height) {
        const float bin_width = spectrum_width / (SPECTRUM_BINS - 1);
        
        for (size_t i = 0; i < SPECTRUM_BINS - 1; ++i) {
            float x1 = spectrum_x + i * bin_width;
            float y1 = spectrum_y + spectrum_height - (self->smoothed_spectrum[i] * spectrum_height);
            float x2 = spectrum_x + (i + 1) * bin_width;
            float y2 = spectrum_y + spectrum_height - (self->smoothed_spectrum[i + 1] * spectrum_height);
            
            // Color based on frequency (low = red, mid = green, high = blue)
            float freq_ratio = (float)i / SPECTRUM_BINS;
            int r = (int)(255 * (1.0f - freq_ratio));
            int g = (int)(255 * (freq_ratio < 0.5f ? 2.0f * freq_ratio : 2.0f * (1.0f - freq_ratio)));
            int b = (int)(255 * freq_ratio);
            
            self->graphics_context->drawLine(clap_jules::graphics::Point(x1, y1), 
                                           clap_jules::graphics::Point(x2, y2),
                                           clap_jules::graphics::Color(r, g, b), 2.0f);
        }
    }
    
    void draw_spectrum_dots(my_plugin_t *self, float spectrum_x, float spectrum_y, float spectrum_width, float spectrum_height) {
        const float bin_width = spectrum_width / SPECTRUM_BINS;
        
        for (size_t i = 0; i < SPECTRUM_BINS; ++i) {
            float x = spectrum_x + i * bin_width;
            float y = spectrum_y + spectrum_height - (self->smoothed_spectrum[i] * spectrum_height);
            
            // Color based on frequency
            float freq_ratio = (float)i / SPECTRUM_BINS;
            int r = (int)(255 * (1.0f - freq_ratio));
            int g = (int)(255 * (freq_ratio < 0.5f ? 2.0f * freq_ratio : 2.0f * (1.0f - freq_ratio)));
            int b = (int)(255 * freq_ratio);
            
            // Size based on magnitude
            float radius = 2.0f + self->smoothed_spectrum[i] * 4.0f;
            
            self->graphics_context->drawCircle(clap_jules::graphics::Point(x, y), radius,
                                             clap_jules::graphics::Color(r, g, b));
        }
    }
    
    void draw_spectrum_bins(my_plugin_t *self, float spectrum_x, float spectrum_y, float spectrum_width, float spectrum_height) {
        const float bin_width = spectrum_width / SPECTRUM_BINS;
        
        for (size_t i = 0; i < SPECTRUM_BINS; ++i) {
            float x = spectrum_x + i * bin_width;
            float height = self->smoothed_spectrum[i] * spectrum_height;
            float y = spectrum_y + spectrum_height - height;
            
            // Color based on frequency
            float freq_ratio = (float)i / SPECTRUM_BINS;
            int r = (int)(255 * (1.0f - freq_ratio));
            int g = (int)(255 * (freq_ratio < 0.5f ? 2.0f * freq_ratio : 2.0f * (1.0f - freq_ratio)));
            int b = (int)(255 * freq_ratio);
            
            self->graphics_context->drawRect(clap_jules::graphics::Rect(x, y, bin_width - 1, height),
                                           clap_jules::graphics::Color(r, g, b));
        }
    }
    
    void draw_spectrum_fill(my_plugin_t *self, float spectrum_x, float spectrum_y, float spectrum_width, float spectrum_height) {
        const float bin_width = spectrum_width / (SPECTRUM_BINS - 1);
        
        // Draw filled area by connecting points with lines to create a filled shape
        std::vector<clap_jules::graphics::Point> points;
        
        // Start from bottom-left
        points.push_back(clap_jules::graphics::Point(spectrum_x, spectrum_y + spectrum_height));
        
        // Add spectrum points
        for (size_t i = 0; i < SPECTRUM_BINS; ++i) {
            float x = spectrum_x + i * bin_width;
            float y = spectrum_y + spectrum_height - (self->smoothed_spectrum[i] * spectrum_height);
            points.push_back(clap_jules::graphics::Point(x, y));
        }
        
        // End at bottom-right
        points.push_back(clap_jules::graphics::Point(spectrum_x + spectrum_width, spectrum_y + spectrum_height));
        
        // Draw filled area with gradient effect
        for (size_t i = 0; i < points.size() - 1; ++i) {
            // Create vertical gradient lines
            float x = points[i].x;
            float y_top = points[i].y;
            float y_bottom = spectrum_y + spectrum_height;
            
            // Color based on frequency
            float freq_ratio = (float)i / points.size();
            int r = (int)(100 * (1.0f - freq_ratio));
            int g = (int)(150 * (freq_ratio < 0.5f ? 2.0f * freq_ratio : 2.0f * (1.0f - freq_ratio)));
            int b = (int)(200 * freq_ratio);
            
            self->graphics_context->drawLine(clap_jules::graphics::Point(x, y_top),
                                           clap_jules::graphics::Point(x, y_bottom),
                                           clap_jules::graphics::Color(r, g, b), 1.0f);
        }
        
        // Draw outline
        for (size_t i = 0; i < SPECTRUM_BINS - 1; ++i) {
            float x1 = spectrum_x + i * bin_width;
            float y1 = spectrum_y + spectrum_height - (self->smoothed_spectrum[i] * spectrum_height);
            float x2 = spectrum_x + (i + 1) * bin_width;
            float y2 = spectrum_y + spectrum_height - (self->smoothed_spectrum[i + 1] * spectrum_height);
            
            self->graphics_context->drawLine(clap_jules::graphics::Point(x1, y1),
                                           clap_jules::graphics::Point(x2, y2),
                                           clap_jules::graphics::Color(255, 255, 255), 2.0f);
        }
    }
}

static void my_plugin_render_content(my_plugin_t *self) {
    if (!self->graphics_context) {
        return;
    }
    
    // Clear background
    self->graphics_context->clear(clap_jules::graphics::Color(20, 20, 30)); // Dark background
    
    // Define layout areas
    const float margin = 20.0f;
    const float control_height = 60.0f;
    const float spectrum_x = margin;
    const float spectrum_y = margin + control_height;
    const float spectrum_width = self->gui_width - 2 * margin;
    const float spectrum_height = self->gui_height - 3 * margin - control_height;
    
    // Draw spectrum area background
    self->graphics_context->drawRect(
        clap_jules::graphics::Rect(spectrum_x, spectrum_y, spectrum_width, spectrum_height),
        clap_jules::graphics::Color(30, 30, 40)
    );
    
    // Draw frequency grid lines
    const int grid_lines = 10;
    for (int i = 0; i <= grid_lines; ++i) {
        float x = spectrum_x + (i * spectrum_width / grid_lines);
        self->graphics_context->drawLine(
            clap_jules::graphics::Point(x, spectrum_y),
            clap_jules::graphics::Point(x, spectrum_y + spectrum_height),
            clap_jules::graphics::Color(50, 50, 60), 1.0f
        );
    }
    
    // Draw magnitude grid lines
    for (int i = 0; i <= 10; ++i) {
        float y = spectrum_y + (i * spectrum_height / 10);
        self->graphics_context->drawLine(
            clap_jules::graphics::Point(spectrum_x, y),
            clap_jules::graphics::Point(spectrum_x + spectrum_width, y),
            clap_jules::graphics::Color(50, 50, 60), 1.0f
        );
    }
    
    // Draw spectrum based on current mode
    switch (self->draw_mode) {
        case SpectrumDrawMode::LINES:
            draw_spectrum_lines(self, spectrum_x, spectrum_y, spectrum_width, spectrum_height);
            break;
        case SpectrumDrawMode::DOTS:
            draw_spectrum_dots(self, spectrum_x, spectrum_y, spectrum_width, spectrum_height);
            break;
        case SpectrumDrawMode::BINS:
            draw_spectrum_bins(self, spectrum_x, spectrum_y, spectrum_width, spectrum_height);
            break;
        case SpectrumDrawMode::FILL:
            draw_spectrum_fill(self, spectrum_x, spectrum_y, spectrum_width, spectrum_height);
            break;
    }
    
    // Draw control panel
    const float button_width = 80.0f;
    const float button_height = 30.0f;
    const float button_spacing = 90.0f;
    const char* mode_names[] = {"Lines", "Dots", "Bins", "Fill"};
    
    for (int i = 0; i < 4; ++i) {
        float btn_x = margin + i * button_spacing;
        float btn_y = margin + 15.0f;
        
        // Button background (highlight if selected)
        clap_jules::graphics::Color btn_color = (self->draw_mode == (SpectrumDrawMode)i) ? 
            clap_jules::graphics::Color(100, 150, 200) : clap_jules::graphics::Color(60, 60, 80);
        
        self->graphics_context->drawRect(
            clap_jules::graphics::Rect(btn_x, btn_y, button_width, button_height),
            btn_color
        );
        
        // Button text
        clap_jules::graphics::Color text_color = (self->draw_mode == (SpectrumDrawMode)i) ? 
            clap_jules::graphics::Color(255, 255, 255) : clap_jules::graphics::Color(200, 200, 200);
        
        self->graphics_context->drawText(
            mode_names[i],
            clap_jules::graphics::Point(btn_x + 5, btn_y + 20),
            text_color, 14.0f
        );
    }
    
    // Draw title and info
    self->graphics_context->drawText("FFT Spectrum Analyzer", 
                                   clap_jules::graphics::Point(margin, spectrum_y + spectrum_height + 30),
                                   clap_jules::graphics::Color(255, 255, 255), 18.0f);
    
    char info_text[128];
    snprintf(info_text, sizeof(info_text), "Mode: %s | FFT Size: %zu | Sample Rate: %.0f Hz", 
             mode_names[(int)self->draw_mode], FFT_SIZE, self->sample_rate);
    self->graphics_context->drawText(info_text,
                                   clap_jules::graphics::Point(margin, spectrum_y + spectrum_height + 55),
                                   clap_jules::graphics::Color(180, 180, 180), 12.0f);
    
    // Finalize rendering
    self->graphics_context->present();
}

static bool my_plugin_present_graphics(my_plugin_t *self) {
    if (!self->graphics_context || !self->gui_created) {
        return false;
    }
    
    // Get the rendered pixel data
    const void* pixel_data = self->graphics_context->getPixelData();
    if (!pixel_data) {
        return false;
    }
    
    int width = self->graphics_context->getWidth();
    int height = self->graphics_context->getHeight();
    
    printf("MyPlugin: Presenting %ux%u graphics buffer to window\n", width, height);
    
#if defined(__linux__) && defined(HAVE_X11)
    // Use X11 renderer if available
    if (self->x11_renderer && self->x11_renderer->isInitialized()) {
        const uint32_t* pixels = static_cast<const uint32_t*>(pixel_data);
        return self->x11_renderer->presentPixelBuffer(pixels, width, height);
    }
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    // Use Win32 renderer if available
    if (self->win32_renderer && self->win32_renderer->isInitialized()) {
        const uint32_t* pixels = static_cast<const uint32_t*>(pixel_data);
        return self->win32_renderer->presentPixelBuffer(pixels, width, height);
    }
#endif
    
    // Fallback: just log that we would present the graphics
    printf("MyPlugin: Would present graphics buffer (no platform renderer available)\n");
    return true;
}

// --- GUI Extension Implementation ---

static bool my_plugin_gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    printf("MyPlugin: GUI - Checking API support: %s (floating: %s)\n", api, is_floating ? "yes" : "no");
    
    // Support the most common windowing APIs
    if (strcmp(api, CLAP_WINDOW_API_X11) == 0) return true;      // Linux X11
    if (strcmp(api, CLAP_WINDOW_API_WIN32) == 0) return true;    // Windows
    if (strcmp(api, CLAP_WINDOW_API_COCOA) == 0) return true;    // macOS
    if (strcmp(api, CLAP_WINDOW_API_WAYLAND) == 0 && is_floating) return true; // Wayland (floating only)
    
    return false;
}

static bool my_plugin_gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating) {
    printf("MyPlugin: GUI - Getting preferred API\n");
    
    // Prefer embedded windows on most platforms
    *is_floating = false;
    
    #ifdef __linux__
        *api = CLAP_WINDOW_API_X11;
    #elif defined(_WIN32)
        *api = CLAP_WINDOW_API_WIN32;
    #elif defined(__APPLE__)
        *api = CLAP_WINDOW_API_COCOA;
    #else
        *api = CLAP_WINDOW_API_X11; // Default fallback
    #endif
    
    printf("MyPlugin: GUI - Preferred API: %s\n", *api);
    return true;
}

static bool my_plugin_gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI - Creating window (API: %s, floating: %s)\n", api ? api : "none", is_floating ? "yes" : "no");
    
    if (self->gui_created) {
        printf("MyPlugin: GUI - Already created\n");
        return false;
    }
    
    // Store GUI settings
    self->gui_api = api;
    self->gui_is_floating = is_floating;
    
    // Create graphics context for the GUI
    self->graphics_context = clap_jules::graphics::createGraphicsContext(self->gui_width, self->gui_height);
    if (!self->graphics_context) {
        printf("MyPlugin: GUI - Failed to create graphics context\n");
        return false;
    }
    
    // Render initial content
    my_plugin_render_content(self);
    
    self->gui_created = true;
    self->needs_redraw = true;
    printf("MyPlugin: GUI - Window created successfully\n");
    return true;
}

static void my_plugin_gui_destroy(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI - Destroying window\n");
    
    if (!self->gui_created) {
        return;
    }
    
    // Clean up graphics context
    self->graphics_context.reset();
    
#if defined(__linux__) && defined(HAVE_X11)
    // Clean up X11 renderer
    self->x11_renderer.reset();
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    // Clean up Win32 renderer
    self->win32_renderer.reset();
#endif
    
    self->gui_created = false;
    self->gui_visible = false;
    self->gui_api = nullptr;
    
    printf("MyPlugin: GUI - Window destroyed\n");
}

static bool my_plugin_gui_set_scale(const clap_plugin_t *plugin, double scale) {
    printf("MyPlugin: GUI - Setting scale: %.2f\n", scale);
    // For now, we ignore scaling but report success
    return true;
}

static bool my_plugin_gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    *width = self->gui_width;
    *height = self->gui_height;
    printf("MyPlugin: GUI - Reporting size: %ux%u\n", *width, *height);
    return true;
}

static bool my_plugin_gui_can_resize(const clap_plugin_t *plugin) {
    printf("MyPlugin: GUI - Can resize: yes\n");
    return true; // Allow resizing
}

static bool my_plugin_gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints) {
    printf("MyPlugin: GUI - Getting resize hints\n");
    hints->can_resize_horizontally = true;
    hints->can_resize_vertically = true;
    hints->preserve_aspect_ratio = false;
    hints->aspect_ratio_width = 4;
    hints->aspect_ratio_height = 3;
    return true;
}

static bool my_plugin_gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    printf("MyPlugin: GUI - Adjusting size from %ux%u", *width, *height);
    
    // Constrain minimum size
    if (*width < 200) *width = 200;
    if (*height < 150) *height = 150;
    
    // Constrain maximum size
    if (*width > 1200) *width = 1200;
    if (*height > 800) *height = 800;
    
    printf(" to %ux%u\n", *width, *height);
    return true;
}

static bool my_plugin_gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI - Setting size: %ux%u\n", width, height);
    
    self->gui_width = width;
    self->gui_height = height;
    
    // Resize Win32 renderer if available
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    if (self->win32_renderer && self->win32_renderer->isInitialized()) {
        self->win32_renderer->resize(width, height);
    }
#endif
    
    // Resize X11 renderer if available
#if defined(__linux__) && defined(HAVE_X11)
    if (self->x11_renderer && self->x11_renderer->isInitialized()) {
        self->x11_renderer->resize(width, height);
    }
#endif
    
    // Recreate graphics context with new size if GUI is created
    if (self->gui_created) {
        self->graphics_context = clap_jules::graphics::createGraphicsContext(width, height);
        if (self->graphics_context) {
            // Re-render content at new size
            my_plugin_render_content(self);
            my_plugin_present_graphics(self);
            self->needs_redraw = true;
            
            // Force immediate refresh for Win32
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
            if (self->win32_renderer && self->win32_renderer->isInitialized()) {
                self->win32_renderer->invalidate();
            }
#endif
        }
    }
    
    return true;
}

static bool my_plugin_gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI - Setting parent window (API: %s)\n", window ? window->api : "null");
    
    if (!window) {
        printf("MyPlugin: GUI - No parent window provided\n");
        return false;
    }
    
    // Store the native window handle for later use
    if (strcmp(window->api, CLAP_WINDOW_API_X11) == 0) {
        self->native_window = (void*)window->x11;
        printf("MyPlugin: GUI - X11 window handle: %lu\n", window->x11);
        
#if defined(__linux__) && defined(HAVE_X11)
        // Initialize X11 renderer
        self->x11_renderer = std::make_unique<clap_jules::graphics::X11Renderer>();
        if (!self->x11_renderer->initialize(window->x11, self->gui_width, self->gui_height)) {
            printf("MyPlugin: GUI - Failed to initialize X11 renderer\n");
            self->x11_renderer.reset();
        } else {
            printf("MyPlugin: GUI - X11 renderer initialized successfully\n");
        }
#endif
        
    } else if (strcmp(window->api, CLAP_WINDOW_API_WIN32) == 0) {
        self->native_window = window->win32;
        printf("MyPlugin: GUI - Win32 window handle set\n");
        
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
        // Initialize Win32 renderer
        self->win32_renderer = std::make_unique<clap_jules::graphics::Win32Renderer>();
        
        // Set up redraw callback to trigger rendering
        self->win32_renderer->setRedrawCallback([self]() {
            if (self && self->gui_created && self->gui_visible && self->graphics_context) {
                my_plugin_render_content(self);
                my_plugin_present_graphics(self);
            }
        });
        
        if (!self->win32_renderer->initialize((HWND)window->win32, self->gui_width, self->gui_height)) {
            printf("MyPlugin: GUI - Failed to initialize Win32 renderer\n");
            self->win32_renderer.reset();
        } else {
            printf("MyPlugin: GUI - Win32 renderer initialized successfully\n");
        }
#endif
    } else if (strcmp(window->api, CLAP_WINDOW_API_COCOA) == 0) {
        self->native_window = window->cocoa;
        printf("MyPlugin: GUI - Cocoa window handle set\n");
    }
    
    // Trigger initial render
    if (self->gui_created) {
        my_plugin_render_content(self);
        my_plugin_present_graphics(self);
        
        // Force immediate refresh for Win32
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
        if (self->win32_renderer && self->win32_renderer->isInitialized()) {
            self->win32_renderer->invalidate();
        }
#endif
    }
    
    printf("MyPlugin: GUI - Parent window set and initial render performed\n");
    return true;
}

static bool my_plugin_gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window) {
    printf("MyPlugin: GUI - Setting transient parent for floating window\n");
    // For floating windows, make the plugin window stay above the parent
    return true;
}

static void my_plugin_gui_suggest_title(const clap_plugin_t *plugin, const char *title) {
    printf("MyPlugin: GUI - Suggested title: %s\n", title ? title : "null");
    // For floating windows, set the window title
}

static bool my_plugin_gui_show(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI - Showing window\n");
    
    if (!self->gui_created) {
        printf("MyPlugin: GUI - Cannot show, window not created\n");
        return false;
    }
    
    self->gui_visible = true;
    
    // Render and present graphics when showing
    my_plugin_render_content(self);
    my_plugin_present_graphics(self);
    
    // Force immediate refresh for all platforms
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    if (self->win32_renderer && self->win32_renderer->isInitialized()) {
        self->win32_renderer->invalidate();
    }
#endif
    
    printf("MyPlugin: GUI - Window is now visible with rendered content\n");
    return true;
}

static bool my_plugin_gui_hide(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI - Hiding window\n");
    
    if (!self->gui_created) {
        return false;
    }
    
    self->gui_visible = false;
    printf("MyPlugin: GUI - Window is now hidden\n");
    return true;
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
