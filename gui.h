#pragma once

#include <clap/clap.h>
#include <clap/ext/gui.h>

#ifdef __cplusplus
extern "C" {
#endif

// GUI-related structure for our plugin
typedef struct {
    // Window API being used
    const char *current_api;
    bool is_floating;
    
    // Window dimensions
    uint32_t width;
    uint32_t height;
    
    // Platform-specific window handles
    union {
        void *win32_hwnd;
        unsigned long x11_window;
        void *cocoa_nsview;
    } window_handle;
    
    // GUI state
    bool is_created;
    bool is_visible;
    double scale_factor;
    
    // GLFW window (for our internal rendering)
    void *glfw_window;
    
} plugin_gui_t;

// Forward declaration of our plugin structure
struct my_plugin;

// GUI extension functions
extern const clap_plugin_gui_t gui_extension;

// GUI initialization and cleanup
bool gui_init();
void gui_cleanup();

// GUI lifecycle functions (for manual use)
void gui_destroy(const clap_plugin_t *plugin);

// Platform-specific implementations
bool gui_create_platform_window(struct my_plugin *plugin, const char *api, bool is_floating);
void gui_destroy_platform_window(struct my_plugin *plugin);
bool gui_set_parent_platform(struct my_plugin *plugin, const clap_window_t *window);
bool gui_show_platform(struct my_plugin *plugin);
bool gui_hide_platform(struct my_plugin *plugin);

#ifdef __cplusplus
}
#endif