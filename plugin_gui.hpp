#pragma once

#include <clap/clap.h>
#include <clap/ext/gui.h>

#ifdef ENABLE_BRISK
#include <brisk/gui/GUIWindow.hpp>
#include <brisk/widgets/Widgets.hpp>
#include <brisk/gui/GUIApplication.hpp>
#include <memory>
#endif

// Plugin GUI implementation
class PluginGUI {
private:
#ifdef ENABLE_BRISK
    std::unique_ptr<Brisk::GUIWindow> m_window;
    bool m_is_floating = false;
    std::string m_api;
#endif
    bool m_is_created = false;
    uint32_t m_width = 400;
    uint32_t m_height = 300;

public:
    PluginGUI();
    ~PluginGUI();

    // CLAP GUI interface methods
    bool is_api_supported(const char* api, bool is_floating);
    bool get_preferred_api(const char** api, bool* is_floating);
    bool create(const char* api, bool is_floating);
    void destroy();
    bool set_scale(double scale);
    bool get_size(uint32_t* width, uint32_t* height);
    bool can_resize();
    bool adjust_size(uint32_t* width, uint32_t* height);
    bool set_size(uint32_t width, uint32_t height);
    bool set_parent(const clap_window_t* window);
    bool set_transient(const clap_window_t* window);
    void suggest_title(const char* title);
    bool show();
    bool hide();

private:
#ifdef ENABLE_BRISK
    void create_brisk_gui();
#endif
};

// C interface functions for CLAP
extern "C" {
    bool plugin_gui_is_api_supported(const clap_plugin_t* plugin, const char* api, bool is_floating);
    bool plugin_gui_get_preferred_api(const clap_plugin_t* plugin, const char** api, bool* is_floating);
    bool plugin_gui_create(const clap_plugin_t* plugin, const char* api, bool is_floating);
    void plugin_gui_destroy(const clap_plugin_t* plugin);
    bool plugin_gui_set_scale(const clap_plugin_t* plugin, double scale);
    bool plugin_gui_get_size(const clap_plugin_t* plugin, uint32_t* width, uint32_t* height);
    bool plugin_gui_can_resize(const clap_plugin_t* plugin);
    bool plugin_gui_adjust_size(const clap_plugin_t* plugin, uint32_t* width, uint32_t* height);
    bool plugin_gui_set_size(const clap_plugin_t* plugin, uint32_t width, uint32_t height);
    bool plugin_gui_set_parent(const clap_plugin_t* plugin, const clap_window_t* window);
    bool plugin_gui_set_transient(const clap_plugin_t* plugin, const clap_window_t* window);
    void plugin_gui_suggest_title(const clap_plugin_t* plugin, const char* title);
    bool plugin_gui_show(const clap_plugin_t* plugin);
    bool plugin_gui_hide(const clap_plugin_t* plugin);
}

// CLAP GUI extension structure
extern const clap_plugin_gui_t clap_plugin_gui_impl;