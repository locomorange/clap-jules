#include "spectrum_gui.h"
#include "my_plugin.h"
#include "spectrum_analyzer.h"
#include <cstring>
#include <cmath>
#include <algorithm>

SpectrumGUI::SpectrumGUI(void* plugin_data)
    : plugin_data_(plugin_data)
    , is_created_(false)
    , is_visible_(false)
    , width_(DEFAULT_WIDTH)
    , height_(DEFAULT_HEIGHT)
    , scale_(1.0)
    , native_window_(nullptr)
{
}

SpectrumGUI::~SpectrumGUI() {
    if (is_created_) {
        destroy();
    }
}

bool SpectrumGUI::is_api_supported(const char* api, bool is_floating) {
    // For this basic implementation, we support only floating windows
    // In a full VSTGUI implementation, we would support embedded windows
    if (is_floating) {
        return true; // Basic floating window support
    }
    
    // Could support platform-specific embedding APIs here
    if (api) {
#ifdef __linux__
        return strcmp(api, CLAP_WINDOW_API_X11) == 0;
#elif defined(_WIN32)
        return strcmp(api, CLAP_WINDOW_API_WIN32) == 0;
#elif defined(__APPLE__)
        return strcmp(api, CLAP_WINDOW_API_COCOA) == 0;
#endif
    }
    
    return false;
}

bool SpectrumGUI::get_preferred_api(const char** api, bool* is_floating) {
    // Prefer floating window for this basic implementation
    *is_floating = true;
    *api = nullptr; // No specific API preference for floating
    return true;
}

bool SpectrumGUI::create(const char* api, bool is_floating) {
    if (is_created_) {
        return false;
    }
    
    // Initialize spectrum display data
    my_plugin_t* plugin = static_cast<my_plugin_t*>(plugin_data_);
    if (plugin && plugin->spectrum_analyzer) {
        display_spectrum_ = plugin->spectrum_analyzer->get_spectrum_data();
        frequency_bins_ = plugin->spectrum_analyzer->get_frequency_bins();
    }
    
    is_created_ = true;
    return true;
}

void SpectrumGUI::destroy() {
    if (!is_created_) {
        return;
    }
    
    if (is_visible_) {
        hide();
    }
    
    native_window_ = nullptr;
    is_created_ = false;
}

bool SpectrumGUI::set_scale(double scale) {
    scale_ = scale;
    return true;
}

bool SpectrumGUI::get_size(uint32_t* width, uint32_t* height) {
    if (!is_created_) {
        return false;
    }
    
    *width = width_;
    *height = height_;
    return true;
}

bool SpectrumGUI::can_resize() {
    return true; // Allow resizing
}

bool SpectrumGUI::set_size(uint32_t width, uint32_t height) {
    if (!is_created_) {
        return false;
    }
    
    width_ = std::max(200u, std::min(1200u, width));   // Constrain width
    height_ = std::max(100u, std::min(600u, height));  // Constrain height
    
    return true;
}

bool SpectrumGUI::set_parent(const clap_window_t* window) {
    if (!is_created_) {
        return false;
    }
    
    native_window_ = window ? window->ptr : nullptr;
    return true;
}

bool SpectrumGUI::show() {
    if (!is_created_ || is_visible_) {
        return false;
    }
    
    // In a real implementation, this would create and show the native window
    // For now, we just mark as visible
    is_visible_ = true;
    
    printf("SpectrumGUI: Window shown (size: %dx%d)\n", width_, height_);
    return true;
}

bool SpectrumGUI::hide() {
    if (!is_created_ || !is_visible_) {
        return false;
    }
    
    is_visible_ = false;
    printf("SpectrumGUI: Window hidden\n");
    return true;
}

void SpectrumGUI::update_spectrum_data() {
    if (!is_created_) {
        return;
    }
    
    my_plugin_t* plugin = static_cast<my_plugin_t*>(plugin_data_);
    if (!plugin || !plugin->spectrum_analyzer) {
        return;
    }
    
    if (plugin->spectrum_analyzer->has_new_data()) {
        display_spectrum_ = plugin->spectrum_analyzer->get_spectrum_data();
        frequency_bins_ = plugin->spectrum_analyzer->get_frequency_bins();
        plugin->spectrum_analyzer->acknowledge_data();
        
        // Trigger GUI update (in real implementation)
        render_spectrum();
    }
}

int SpectrumGUI::get_drawing_style() const {
    my_plugin_t* plugin = static_cast<my_plugin_t*>(plugin_data_);
    if (!plugin) {
        return STYLE_LINES;
    }
    return static_cast<int>(plugin->spectrum_drawing_style);
}

void SpectrumGUI::render_spectrum() {
    if (!is_visible_ || display_spectrum_.empty()) {
        return;
    }
    
    // In a real implementation, this would render to the actual GUI surface
    // For now, we just call the appropriate drawing method based on style
    
    int style = get_drawing_style();
    switch (style) {
        case STYLE_LINES:
            draw_lines(display_spectrum_);
            break;
        case STYLE_DOTS:
            draw_dots(display_spectrum_);
            break;
        case STYLE_BINS:
            draw_bins(display_spectrum_);
            break;
        case STYLE_FILLS:
            draw_fills(display_spectrum_);
            break;
        default:
            draw_lines(display_spectrum_);
            break;
    }
}

void SpectrumGUI::draw_lines(const std::vector<float>& spectrum) {
    // Placeholder for line drawing
    // In real implementation: draw connected lines between spectrum points
    printf("Drawing spectrum as lines (%zu points)\n", spectrum.size());
}

void SpectrumGUI::draw_dots(const std::vector<float>& spectrum) {
    // Placeholder for dot drawing
    // In real implementation: draw individual dots at spectrum points
    printf("Drawing spectrum as dots (%zu points)\n", spectrum.size());
}

void SpectrumGUI::draw_bins(const std::vector<float>& spectrum) {
    // Placeholder for bin drawing
    // In real implementation: draw vertical bars for each frequency bin
    printf("Drawing spectrum as bins (%zu bins)\n", spectrum.size());
}

void SpectrumGUI::draw_fills(const std::vector<float>& spectrum) {
    // Placeholder for filled drawing
    // In real implementation: draw filled area under the spectrum curve
    printf("Drawing spectrum as fills (%zu points)\n", spectrum.size());
}