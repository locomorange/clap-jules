#include "my_plugin.h"
#include "plugin_gui.h"
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc
#include <clap/ext/gui.h>
#include <clap/ext/audio-ports.h>
#include <clap/plugin-features.h>

// Include Qt headers at the top
#include <QtWidgets/QApplication>
#include <QtCore/QCoreApplication>
#include <QtCore/QString>
#include <QtGui/QWindow>
#include <QtWidgets/QWidget>

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

// --- Plugin Descriptor ---
// Features array for the plugin descriptor
static const char *const plugin_features[] = {CLAP_PLUGIN_FEATURE_AUDIO_EFFECT, nullptr};

static const clap_plugin_descriptor_t my_plugin_descriptor = {
    CLAP_VERSION,
    "com.yourcompany.qtclapdemo", // id - more unique
    "Qt CLAP Demo Plugin", // name
    "Your Company",         // vendor
    "https://yourcompany.com",  // url
    "https://yourcompany.com/manual", // manual_url
    "https://yourcompany.com/support", // support_url
    "1.0.0",                // version
    "A CLAP audio plugin with Qt GUI demonstration.", // description
    plugin_features, // features
};


// --- Plugin Implementation ---
static bool my_plugin_init(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Initializing plugin\n");
    
    // Initialize GUI state
    self->gui_widget = nullptr;
    self->gui_created = false;
    
    // Don't initialize Qt here - do it only when GUI is needed
    
    return true;
}

static void my_plugin_destroy(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Destroying plugin\n");
    
    // Clean up GUI
    if (self->gui_widget) {
        delete self->gui_widget;
        self->gui_widget = nullptr;
    }
    
    // Free any other resources allocated in init
}

static bool my_plugin_activate(const struct clap_plugin *plugin, double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count) {
    printf("MyPlugin: Activating plugin (Sample Rate: %.2f, Min Frames: %u, Max Frames: %u)\n", sample_rate, min_frames_count, max_frames_count);
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

static clap_process_status my_plugin_process(const struct clap_plugin *plugin, const clap_process_t *process) {
    // Simple passthrough audio processing
    if (process->audio_outputs_count > 0 && process->audio_inputs_count > 0) {
        clap_audio_buffer_t *out_buf = &process->audio_outputs[0];
        const clap_audio_buffer_t *in_buf = &process->audio_inputs[0];

        // Ensure we have valid buffers and matching channel counts
        if (out_buf->data32 && in_buf->data32 && 
            out_buf->channel_count >= 2 && in_buf->channel_count >= 2) {
            
            // Simple stereo passthrough
            for (uint32_t i = 0; i < process->frames_count; ++i) {
                out_buf->data32[0][i] = in_buf->data32[0][i]; // Left channel
                out_buf->data32[1][i] = in_buf->data32[1][i]; // Right channel
            }
        } else {
            // Clear output if no valid input
            if (out_buf->data32 && out_buf->channel_count >= 2) {
                for (uint32_t i = 0; i < process->frames_count; ++i) {
                    out_buf->data32[0][i] = 0.0f; // Left channel
                    out_buf->data32[1][i] = 0.0f; // Right channel
                }
            }
        }
    }
    
    return CLAP_PROCESS_CONTINUE;
}

// GUI Extension Implementation
static bool my_plugin_gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    printf("MyPlugin: GUI is_api_supported called with api: %s, floating: %s\n", api, is_floating ? "true" : "false");
    return strcmp(api, CLAP_WINDOW_API_WIN32) == 0 || 
           strcmp(api, CLAP_WINDOW_API_COCOA) == 0 || 
           strcmp(api, CLAP_WINDOW_API_X11) == 0 ||
           strcmp(api, CLAP_WINDOW_API_WAYLAND) == 0;
}

static bool my_plugin_gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating) {
    printf("MyPlugin: GUI get_preferred_api called\n");
#ifdef _WIN32
    *api = CLAP_WINDOW_API_WIN32;
#elif defined(__APPLE__)
    *api = CLAP_WINDOW_API_COCOA;
#else
    *api = CLAP_WINDOW_API_X11;
#endif
    *is_floating = false;
    return true;
}

static bool my_plugin_gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI create called with api: %s, floating: %s\n", api, is_floating ? "true" : "false");
    
    if (!self->gui_widget && !self->gui_created) {
        // Lazy Qt initialization only when GUI is actually needed
        if (!QApplication::instance()) {
            static int argc = 1;
            static char *argv[] = {(char*)"my_clap_plugin", nullptr};
            
            // Set platform to offscreen if no display is available
            const char* display = getenv("DISPLAY");
            if (!display || strlen(display) == 0) {
                // Headless environment - use offscreen platform
                qputenv("QT_QPA_PLATFORM", "offscreen");
                printf("MyPlugin: Using offscreen Qt platform for headless environment\n");
            }
            
            try {
                static QApplication app(argc, argv);
                printf("MyPlugin: Qt application initialized successfully\n");
            } catch (...) {
                printf("MyPlugin: Failed to initialize Qt application\n");
                return false;
            }
        }
        
        self->gui_widget = new PluginWidget();
        self->gui_widget->setHost(self->host);
        self->gui_created = true;
        printf("MyPlugin: GUI widget created successfully\n");
        return true;
    }
    
    return false;
}

static void my_plugin_gui_destroy(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI destroy called\n");
    
    if (self->gui_widget) {
        delete self->gui_widget;
        self->gui_widget = nullptr;
        self->gui_created = false;
        printf("MyPlugin: GUI widget destroyed\n");
    }
}

static bool my_plugin_gui_set_scale(const clap_plugin_t *plugin, double scale) {
    printf("MyPlugin: GUI set_scale called with scale: %f\n", scale);
    return true;
}

static bool my_plugin_gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI get_size called\n");
    
    if (self->gui_widget) {
        *width = 300;
        *height = 200;
        return true;
    }
    
    *width = 300;
    *height = 200;
    return true;
}

static bool my_plugin_gui_can_resize(const clap_plugin_t *plugin) {
    printf("MyPlugin: GUI can_resize called\n");
    return false;
}

static bool my_plugin_gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints) {
    printf("MyPlugin: GUI get_resize_hints called\n");
    return false;
}

static bool my_plugin_gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    printf("MyPlugin: GUI adjust_size called\n");
    *width = 300;
    *height = 200;
    return true;
}

static bool my_plugin_gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height) {
    printf("MyPlugin: GUI set_size called with width: %u, height: %u\n", width, height);
    return true;
}

static bool my_plugin_gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI set_parent called\n");
    
    if (self->gui_widget && window) {
        // Embed the Qt widget into the host window
        WId parentWinId = 0;
        
#ifdef _WIN32
        if (strcmp(window->api, CLAP_WINDOW_API_WIN32) == 0) {
            parentWinId = (WId)window->win32;
        }
#elif defined(__APPLE__)
        if (strcmp(window->api, CLAP_WINDOW_API_COCOA) == 0) {
            parentWinId = (WId)window->cocoa;
        }
#else
        if (strcmp(window->api, CLAP_WINDOW_API_X11) == 0) {
            parentWinId = (WId)window->x11;
        }
#endif
        
        if (parentWinId) {
            self->gui_widget->winId(); // Ensure native window is created
            QWindow *parentWindow = QWindow::fromWinId(parentWinId);
            if (parentWindow) {
                QWidget *parentWidget = QWidget::createWindowContainer(parentWindow);
                self->gui_widget->setParent(parentWidget);
            }
            self->gui_widget->show();
            printf("MyPlugin: GUI widget parented and shown\n");
            return true;
        }
    }
    
    return false;
}

static bool my_plugin_gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window) {
    printf("MyPlugin: GUI set_transient called\n");
    return false;
}

static void my_plugin_gui_suggest_title(const clap_plugin_t *plugin, const char *title) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI suggest_title called with title: %s\n", title);
    
    if (self->gui_widget) {
        self->gui_widget->setWindowTitle(QString::fromUtf8(title));
    }
}

static bool my_plugin_gui_show(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI show called\n");
    
    if (self->gui_widget) {
        self->gui_widget->show();
        return true;
    }
    
    return false;
}

static bool my_plugin_gui_hide(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI hide called\n");
    
    if (self->gui_widget) {
        self->gui_widget->hide();
        return true;
    }
    
    return false;
}

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

// Audio Ports Extension Implementation
static uint32_t my_plugin_audio_ports_count(const clap_plugin_t *plugin, bool is_input) {
    // Return 1 input port and 1 output port
    return 1;
}

static bool my_plugin_audio_ports_get(const clap_plugin_t *plugin, uint32_t index, bool is_input, clap_audio_port_info_t *info) {
    if (index != 0) return false;
    
    if (is_input) {
        info->id = 0;
        strcpy(info->name, "Audio Input");
        info->channel_count = 2;
        info->flags = CLAP_AUDIO_PORT_IS_MAIN;
        info->port_type = CLAP_PORT_STEREO;
        info->in_place_pair = 0; // Paired with output port 0
    } else {
        info->id = 0;
        strcpy(info->name, "Audio Output");
        info->channel_count = 2;
        info->flags = CLAP_AUDIO_PORT_IS_MAIN;
        info->port_type = CLAP_PORT_STEREO;
        info->in_place_pair = 0; // Paired with input port 0
    }
    
    return true;
}

static const clap_plugin_audio_ports_t my_plugin_audio_ports = {
    my_plugin_audio_ports_count,
    my_plugin_audio_ports_get,
};

static const void *my_plugin_get_extension(const struct clap_plugin *plugin, const char *id) {
    printf("MyPlugin: Host requesting extension: %s\n", id);
    
    if (strcmp(id, CLAP_EXT_GUI) == 0) {
        printf("MyPlugin: Returning GUI extension\n");
        return &my_plugin_gui;
    }
    
    if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) {
        printf("MyPlugin: Returning Audio Ports extension\n");
        return &my_plugin_audio_ports;
    }
    
    return NULL;
}

static void my_plugin_on_main_thread(const struct clap_plugin *plugin) {
    // Called by the host to perform tasks that must run on the main thread.
    // printf("MyPlugin: on_main_thread called\n");
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
    printf("MyPlugin: create_plugin called with ID: %s (expected: %s)\n", plugin_id, my_plugin_descriptor.id);
    
    if (strcmp(plugin_id, my_plugin_descriptor.id) != 0) {
        fprintf(stderr, "MyPlugin: Error - incorrect plugin ID requested: %s\n", plugin_id);
        return NULL;
    }

    my_plugin_t *self = (my_plugin_t *)calloc(1, sizeof(my_plugin_t));
    if (!self) {
        fprintf(stderr, "MyPlugin: Error - failed to allocate memory for plugin instance\n");
        return NULL;
    }

    self->host = host;
    self->gui_widget = nullptr;
    self->gui_created = false;
    
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
