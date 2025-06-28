#include "plugin_gui.hpp"
#include "my_plugin.h"
#include <cstring>
#include <stdio.h>

#ifdef ENABLE_BRISK
using namespace Brisk;
#endif

PluginGUI::PluginGUI() {
    printf("PluginGUI: Constructor called\n");
}

PluginGUI::~PluginGUI() {
    printf("PluginGUI: Destructor called\n");
    if (m_is_created) {
        destroy();
    }
}

bool PluginGUI::is_api_supported(const char* api, bool is_floating) {
    printf("PluginGUI: is_api_supported called - api: %s, floating: %s\n", 
           api ? api : "null", is_floating ? "true" : "false");
    
#ifdef ENABLE_BRISK
    // Brisk supports native window creation
    if (is_floating) {
        return true; // Brisk can create floating windows
    }
    // For embedded windows, check specific platform APIs
    if (api) {
#ifdef __linux__
        return strcmp(api, CLAP_WINDOW_API_X11) == 0;
#elif defined(_WIN32)
        return strcmp(api, CLAP_WINDOW_API_WIN32) == 0;
#elif defined(__APPLE__)
        return strcmp(api, CLAP_WINDOW_API_COCOA) == 0;
#endif
    }
#endif
    return false;
}

bool PluginGUI::get_preferred_api(const char** api, bool* is_floating) {
    printf("PluginGUI: get_preferred_api called\n");
    
#ifdef ENABLE_BRISK
    // Prefer floating windows for simplicity with Brisk
    *is_floating = true;
    *api = nullptr; // No specific API needed for floating
    return true;
#endif
    return false;
}

bool PluginGUI::create(const char* api, bool is_floating) {
    printf("PluginGUI: create called - api: %s, floating: %s\n", 
           api ? api : "null", is_floating ? "true" : "false");
    
    if (m_is_created) {
        printf("PluginGUI: Already created\n");
        return false;
    }

#ifdef ENABLE_BRISK
    m_is_floating = is_floating;
    m_api = api ? api : "";
    
    try {
        create_brisk_gui();
        m_is_created = true;
        printf("PluginGUI: Created successfully with Brisk\n");
        return true;
    } catch (const std::exception& e) {
        printf("PluginGUI: Error creating Brisk GUI: %s\n", e.what());
        return false;
    }
#else
    printf("PluginGUI: Brisk not enabled\n");
    return false;
#endif
}

void PluginGUI::destroy() {
    printf("PluginGUI: destroy called\n");
    
    if (!m_is_created) {
        return;
    }

#ifdef ENABLE_BRISK
    if (m_window) {
        m_window->close();
        m_window.reset();
    }
#endif
    
    m_is_created = false;
    printf("PluginGUI: Destroyed\n");
}

bool PluginGUI::set_scale(double scale) {
    printf("PluginGUI: set_scale called - scale: %f\n", scale);
    // Brisk handles scaling automatically
    return true;
}

bool PluginGUI::get_size(uint32_t* width, uint32_t* height) {
    printf("PluginGUI: get_size called\n");
    
    if (width) *width = m_width;
    if (height) *height = m_height;
    
    printf("PluginGUI: Size - width: %u, height: %u\n", m_width, m_height);
    return true;
}

bool PluginGUI::can_resize() {
    printf("PluginGUI: can_resize called\n");
    return true;
}

bool PluginGUI::adjust_size(uint32_t* width, uint32_t* height) {
    printf("PluginGUI: adjust_size called - width: %u, height: %u\n", 
           width ? *width : 0, height ? *height : 0);
    
    // Ensure minimum size
    if (width && *width < 200) *width = 200;
    if (height && *height < 150) *height = 150;
    
    return true;
}

bool PluginGUI::set_size(uint32_t width, uint32_t height) {
    printf("PluginGUI: set_size called - width: %u, height: %u\n", width, height);
    
    m_width = width;
    m_height = height;
    
#ifdef ENABLE_BRISK
    if (m_window && m_is_created) {
        m_window->setSize(Size{static_cast<float>(width), static_cast<float>(height)});
    }
#endif
    
    return true;
}

bool PluginGUI::set_parent(const clap_window_t* window) {
    printf("PluginGUI: set_parent called\n");
    // For embedded windows - not implemented yet
    return false;
}

bool PluginGUI::set_transient(const clap_window_t* window) {
    printf("PluginGUI: set_transient called\n");
    // For floating windows to stay above parent
    return true;
}

void PluginGUI::suggest_title(const char* title) {
    printf("PluginGUI: suggest_title called - title: %s\n", title ? title : "null");
    
#ifdef ENABLE_BRISK
    if (m_window && title) {
        m_window->setTitle(title);
    }
#endif
}

bool PluginGUI::show() {
    printf("PluginGUI: show called\n");
    
    if (!m_is_created) {
        printf("PluginGUI: Not created yet\n");
        return false;
    }

#ifdef ENABLE_BRISK
    if (m_window) {
        m_window->show();
        printf("PluginGUI: Window shown\n");
        return true;
    }
#endif
    
    return false;
}

bool PluginGUI::hide() {
    printf("PluginGUI: hide called\n");
    
#ifdef ENABLE_BRISK
    if (m_window) {
        m_window->hide();
        printf("PluginGUI: Window hidden\n");
        return true;
    }
#endif
    
    return false;
}

#ifdef ENABLE_BRISK
void PluginGUI::create_brisk_gui() {
    printf("PluginGUI: Creating Brisk GUI\n");
    
    // Create a simple component with some widgets
    auto component = rcnew Component{
        flexColumn,
        {
            rcnew Text("CLAP Plugin with Brisk UI"),
            rcnew Button("Test Button") | bindMethod(&PluginGUI::show, this),
            rcnew Slider(0.0f, 1.0f, 0.5f),
            rcnew Text("Volume Control"),
            rcnew CheckBox("Enable Effect"),
        }
    };
    
    // Create the GUI window
    m_window = std::make_unique<GUIWindow>(component);
    m_window->setTitle("My CLAP Plugin");
    m_window->setSize(Size{static_cast<float>(m_width), static_cast<float>(m_height)});
    
    printf("PluginGUI: Brisk GUI created\n");
}
#endif

// C interface implementations
extern "C" {
    
bool plugin_gui_is_api_supported(const clap_plugin_t* plugin, const char* api, bool is_floating) {
    my_plugin_t* self = (my_plugin_t*)plugin->plugin_data;
    if (!self || !self->gui) return false;
    return self->gui->is_api_supported(api, is_floating);
}

bool plugin_gui_get_preferred_api(const clap_plugin_t* plugin, const char** api, bool* is_floating) {
    my_plugin_t* self = (my_plugin_t*)plugin->plugin_data;
    if (!self || !self->gui) return false;
    return self->gui->get_preferred_api(api, is_floating);
}

bool plugin_gui_create(const clap_plugin_t* plugin, const char* api, bool is_floating) {
    my_plugin_t* self = (my_plugin_t*)plugin->plugin_data;
    if (!self || !self->gui) return false;
    return self->gui->create(api, is_floating);
}

void plugin_gui_destroy(const clap_plugin_t* plugin) {
    my_plugin_t* self = (my_plugin_t*)plugin->plugin_data;
    if (!self || !self->gui) return;
    self->gui->destroy();
}

bool plugin_gui_set_scale(const clap_plugin_t* plugin, double scale) {
    my_plugin_t* self = (my_plugin_t*)plugin->plugin_data;
    if (!self || !self->gui) return false;
    return self->gui->set_scale(scale);
}

bool plugin_gui_get_size(const clap_plugin_t* plugin, uint32_t* width, uint32_t* height) {
    my_plugin_t* self = (my_plugin_t*)plugin->plugin_data;
    if (!self || !self->gui) return false;
    return self->gui->get_size(width, height);
}

bool plugin_gui_can_resize(const clap_plugin_t* plugin) {
    my_plugin_t* self = (my_plugin_t*)plugin->plugin_data;
    if (!self || !self->gui) return false;
    return self->gui->can_resize();
}

bool plugin_gui_adjust_size(const clap_plugin_t* plugin, uint32_t* width, uint32_t* height) {
    my_plugin_t* self = (my_plugin_t*)plugin->plugin_data;
    if (!self || !self->gui) return false;
    return self->gui->adjust_size(width, height);
}

bool plugin_gui_set_size(const clap_plugin_t* plugin, uint32_t width, uint32_t height) {
    my_plugin_t* self = (my_plugin_t*)plugin->plugin_data;
    if (!self || !self->gui) return false;
    return self->gui->set_size(width, height);
}

bool plugin_gui_set_parent(const clap_plugin_t* plugin, const clap_window_t* window) {
    my_plugin_t* self = (my_plugin_t*)plugin->plugin_data;
    if (!self || !self->gui) return false;
    return self->gui->set_parent(window);
}

bool plugin_gui_set_transient(const clap_plugin_t* plugin, const clap_window_t* window) {
    my_plugin_t* self = (my_plugin_t*)plugin->plugin_data;
    if (!self || !self->gui) return false;
    return self->gui->set_transient(window);
}

void plugin_gui_suggest_title(const clap_plugin_t* plugin, const char* title) {
    my_plugin_t* self = (my_plugin_t*)plugin->plugin_data;
    if (!self || !self->gui) return;
    self->gui->suggest_title(title);
}

bool plugin_gui_show(const clap_plugin_t* plugin) {
    my_plugin_t* self = (my_plugin_t*)plugin->plugin_data;
    if (!self || !self->gui) return false;
    return self->gui->show();
}

bool plugin_gui_hide(const clap_plugin_t* plugin) {
    my_plugin_t* self = (my_plugin_t*)plugin->plugin_data;
    if (!self || !self->gui) return false;
    return self->gui->hide();
}

} // extern "C"

// CLAP GUI extension implementation
const clap_plugin_gui_t clap_plugin_gui_impl = {
    plugin_gui_is_api_supported,
    plugin_gui_get_preferred_api,
    plugin_gui_create,
    plugin_gui_destroy,
    plugin_gui_set_scale,
    plugin_gui_get_size,
    plugin_gui_can_resize,
    nullptr, // get_resize_hints - not implemented
    plugin_gui_adjust_size,
    plugin_gui_set_size,
    plugin_gui_set_parent,
    plugin_gui_set_transient,
    plugin_gui_suggest_title,
    plugin_gui_show,
    plugin_gui_hide,
};