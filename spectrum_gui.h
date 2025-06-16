#pragma once

#include <clap/ext/gui.h>
#include <vector>

/**
 * Basic GUI implementation for the spectrum analyzer
 * This is a minimal implementation that could be replaced with VSTGUI later
 */
class SpectrumGUI {
public:
    SpectrumGUI(void* plugin_data);
    ~SpectrumGUI();
    
    // CLAP GUI interface methods
    bool is_api_supported(const char* api, bool is_floating);
    bool get_preferred_api(const char** api, bool* is_floating);
    bool create(const char* api, bool is_floating);
    void destroy();
    bool set_scale(double scale);
    bool get_size(uint32_t* width, uint32_t* height);
    bool can_resize();
    bool set_size(uint32_t width, uint32_t height);
    bool set_parent(const clap_window_t* window);
    bool show();
    bool hide();
    
    // Update spectrum data for display
    void update_spectrum_data();
    
    // Get drawing style from plugin
    int get_drawing_style() const;

    // Public methods for testing
    void test_render_spectrum() { render_spectrum(); }
    void test_draw_bins(const std::vector<float>& spectrum) { draw_bins(spectrum); }

private:
    void* plugin_data_;
    bool is_created_;
    bool is_visible_;
    uint32_t width_;
    uint32_t height_;
    double scale_;
    
    // Platform-specific GUI context (would be VSTGUI frame in full implementation)
    void* native_window_;
    
    // Spectrum display data
    std::vector<float> display_spectrum_;
    std::vector<float> frequency_bins_;
    
    void render_spectrum();
    void draw_lines(const std::vector<float>& spectrum);
    void draw_dots(const std::vector<float>& spectrum);
    void draw_bins(const std::vector<float>& spectrum);
    void draw_fills(const std::vector<float>& spectrum);
    
    static constexpr uint32_t DEFAULT_WIDTH = 600;
    static constexpr uint32_t DEFAULT_HEIGHT = 300;
};