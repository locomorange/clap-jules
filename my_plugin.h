#pragma once

#include <clap/clap.h>
#include <clap/ext/gui.h>
#include <memory>
#include <vector>
#include <complex>
#include "graphics/skia_graphics.h"

#if defined(__linux__) && defined(HAVE_X11)
#include "graphics/x11_renderer.h"
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include "graphics/win32_renderer.h"
#endif

// Spectrum analyzer parameters
constexpr size_t FFT_SIZE = 1024;
constexpr size_t SPECTRUM_BINS = FFT_SIZE / 2;

// Drawing modes for spectrum visualization
enum class SpectrumDrawMode {
    LINES = 0,
    DOTS = 1,
    BINS = 2,
    FILL = 3
};

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    
    // GUI-related data
    bool gui_created = false;
    bool gui_visible = false;
    uint32_t gui_width = 0;
    uint32_t gui_height = 0;
    const char* gui_api = nullptr;
    bool gui_is_floating = false;
    
    // Graphics context for rendering
    std::unique_ptr<clap_jules::graphics::GraphicsContext> graphics_context = nullptr;
    
    // Platform-specific window handle
    void* native_window = nullptr;
    
    // Platform-specific renderer
#if defined(__linux__) && defined(HAVE_X11)
    std::unique_ptr<clap_jules::graphics::X11Renderer> x11_renderer = nullptr;
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    std::unique_ptr<clap_jules::graphics::Win32Renderer> win32_renderer = nullptr;
#endif
    
    // Render callback data
    bool needs_redraw = false;
    
    // Audio processing data
    double sample_rate = 44100.0;
    
    // FFT and spectrum analyzer data
    std::vector<float> fft_input_buffer;
    std::vector<std::complex<float>> fft_buffer;
    std::vector<float> spectrum_magnitudes;
    std::vector<float> smoothed_spectrum;
    size_t input_buffer_pos = 0;
    SpectrumDrawMode draw_mode = SpectrumDrawMode::LINES;
    
    // Smoothing factor for spectrum display (0.0 to 1.0)
    float spectrum_smoothing = 0.7f;
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
