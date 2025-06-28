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
    
    // Support floating windows with simple GUI
    if (is_floating) {
        return true;
    }
    
    // Support embedded windows based on platform
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

bool PluginGUI::get_preferred_api(const char** api, bool* is_floating) {
    printf("PluginGUI: get_preferred_api called\n");
    
    // Prefer floating windows for simplicity
    *is_floating = true;
    *api = nullptr; // No specific API needed for floating
    return true;
}

bool PluginGUI::create(const char* api, bool is_floating) {
    printf("PluginGUI: create called - api: %s, floating: %s\n", 
           api ? api : "null", is_floating ? "true" : "false");
    
    if (m_is_created) {
        printf("PluginGUI: Already created\n");
        return false;
    }

    m_is_floating = is_floating;
    m_api = api ? api : "";

#ifdef ENABLE_BRISK
    // Try Brisk first
    try {
        create_brisk_gui();
        m_use_brisk = true;
        m_is_created = true;
        printf("PluginGUI: Created successfully with Brisk\n");
        return true;
    } catch (const std::exception& e) {
        printf("PluginGUI: Error creating Brisk GUI: %s, falling back to simple GUI\n", e.what());
    }
#endif

    // Fallback to simple GUI
    printf("PluginGUI: Creating simple GUI\n");
    m_simple_gui = std::make_unique<SimpleGUI>();
    if (m_simple_gui->create()) {
        m_use_brisk = false;
        m_is_created = true;
        printf("PluginGUI: Created successfully with simple GUI\n");
        return true;
    }
    
    printf("PluginGUI: Failed to create GUI\n");
    return false;
}

void PluginGUI::destroy() {
    printf("PluginGUI: destroy called\n");
    
    if (!m_is_created) {
        return;
    }

#ifdef ENABLE_BRISK
    if (m_use_brisk && m_brisk_window) {
        m_brisk_window->close();
        m_brisk_window.reset();
    }
#endif

    if (!m_use_brisk && m_simple_gui) {
        m_simple_gui->destroy();
        m_simple_gui.reset();
    }
    
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
    
    if (!m_use_brisk && m_simple_gui) {
        return m_simple_gui->get_size(width, height);
    }
    
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
    if (m_use_brisk && m_brisk_window && m_is_created) {
        m_brisk_window->setSize(Size{static_cast<float>(width), static_cast<float>(height)});
    }
#endif

    if (!m_use_brisk && m_simple_gui) {
        return m_simple_gui->set_size(width, height);
    }
    
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
    if (m_use_brisk && m_brisk_window && title) {
        m_brisk_window->setTitle(title);
    }
#endif

    if (!m_use_brisk && m_simple_gui && title) {
        m_simple_gui->set_title(title);
    }
}

bool PluginGUI::show() {
    printf("PluginGUI: show called\n");
    
    if (!m_is_created) {
        printf("PluginGUI: Not created yet\n");
        return false;
    }

#ifdef ENABLE_BRISK
    if (m_use_brisk && m_brisk_window) {
        m_brisk_window->show();
        printf("PluginGUI: Brisk window shown\n");
        return true;
    }
#endif

    if (!m_use_brisk && m_simple_gui) {
        bool result = m_simple_gui->show();
        printf("PluginGUI: Simple GUI window shown: %s\n", result ? "success" : "failed");
        return result;
    }
    
    return false;
}

bool PluginGUI::hide() {
    printf("PluginGUI: hide called\n");
    
#ifdef ENABLE_BRISK
    if (m_use_brisk && m_brisk_window) {
        m_brisk_window->hide();
        printf("PluginGUI: Brisk window hidden\n");
        return true;
    }
#endif

    if (!m_use_brisk && m_simple_gui) {
        bool result = m_simple_gui->hide();
        printf("PluginGUI: Simple GUI window hidden: %s\n", result ? "success" : "failed");
        return result;
    }
    
    return false;
}

#ifdef ENABLE_BRISK
void PluginGUI::create_brisk_gui() {
    printf("PluginGUI: Creating Brisk GUI\n");
    
    // Initialize Brisk application if not already done
    static bool brisk_initialized = false;
    if (!brisk_initialized) {
        // Initialize Brisk application
        printf("PluginGUI: Initializing Brisk application\n");
        brisk_initialized = true;
    }
    
    // Create a simple component with some widgets
    auto component = rcnew Component{
        flexColumn,
        {
            rcnew Text("CLAP Plugin with Brisk UI"),
            rcnew Button("Test Button"),
            rcnew Slider(0.0f, 1.0f, 0.5f),
            rcnew Text("Volume Control"),
            rcnew CheckBox("Enable Effect"),
        }
    };
    
    // Create the GUI window
    m_brisk_window = std::make_unique<GUIWindow>(component);
    m_brisk_window->setTitle("My CLAP Plugin");
    m_brisk_window->setSize(Size{static_cast<float>(m_width), static_cast<float>(m_height)});
    
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