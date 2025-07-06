#include "my_plugin.h"
#include <clap/ext/gui.h>
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc
#include <algorithm> // For std::min
#include <exception> // For exception handling

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

// --- GUI Extension Forward Declarations ---
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

// --- Plugin Descriptor ---
// Features array for the plugin descriptor
static const char *const plugin_features[] = {"audio-effect", nullptr};

static const clap_plugin_descriptor_t my_plugin_descriptor = {
    CLAP_VERSION,
    "com.example.myplugin", // id
    "My First CLAP Plugin", // name
    "My Company",           // vendor
    "https://example.com",  // url
    "https://example.com/bugtracker", // manual_url
    "https://example.com/support",    // support_url
    "0.0.1",                // version
    "A simple example CLAP audio plugin.", // description
    plugin_features, // features
    // CLAP_PLUGIN_FEATURE_AUDIO_EFFECT, // Example if using clap_plugin_features.h
};

// --- GUI Extension Implementation ---
static const clap_plugin_gui_t my_plugin_gui_extension = {
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


// --- Plugin Implementation ---
static bool my_plugin_init(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Initializing plugin\n");
    
    try {
        // Create dependency injection container
        auto injector = plugin::CreateDIContainer();
        
        // Initialize MVVM components
        self->model = injector.create<std::shared_ptr<plugin::PluginModel>>();
        self->processor = injector.create<std::shared_ptr<plugin::AudioProcessor>>();
        self->viewmodel = injector.create<std::shared_ptr<plugin::PluginViewModel>>();
        self->ui_view = injector.create<std::shared_ptr<plugin::BriskUIView>>();
        
        // Initialize plugin state
        self->sample_rate = 44100.0;
        self->is_processing = false;
        
        // Initialize GUI state
        self->gui_created = false;
        self->gui_visible = false;
        self->gui_width = 400;
        self->gui_height = 300;
        self->parent_window = nullptr;
        
        printf("MyPlugin: MVVM components initialized successfully\n");
        return true;
    } catch (const std::exception& e) {
        printf("MyPlugin: Failed to initialize MVVM components: %s\n", e.what());
        return false;
    }
}

static void my_plugin_destroy(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Destroying plugin\n");
    
    // Clean up MVVM components
    if (self) {
        self->ui_view.reset();
        self->viewmodel.reset();
        self->processor.reset();
        self->model.reset();
    }
}

static bool my_plugin_activate(const struct clap_plugin *plugin, double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Activating plugin (Sample Rate: %.2f, Min Frames: %u, Max Frames: %u)\n", sample_rate, min_frames_count, max_frames_count);
    
    // Store activation parameters
    self->sample_rate = sample_rate;
    self->min_frames = min_frames_count;
    self->max_frames = max_frames_count;
    
    // Initialize audio processor with sample rate
    if (self->processor) {
        self->processor->Initialize(sample_rate);
        printf("MyPlugin: Audio processor initialized with sample rate %.2f\n", sample_rate);
    }
    
    return true;
}

static void my_plugin_deactivate(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Deactivating plugin\n");
    self->is_processing = false;
}

static bool my_plugin_start_processing(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Starting processing\n");
    self->is_processing = true;
    return true;
}

static void my_plugin_stop_processing(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Stopping processing\n");
    self->is_processing = false;
}

static void my_plugin_reset(const struct clap_plugin *plugin) {
    printf("MyPlugin: Resetting plugin\n");
    // Reset plugin state (e.g., clear buffers, reset parameters)
}

static clap_process_status my_plugin_process(const struct clap_plugin *plugin, const clap_process_t *process) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    if (!self->is_processing || !self->processor) {
        return CLAP_PROCESS_CONTINUE;
    }
    
    // Process audio from input to output with KFR low-pass filter
    if (process->audio_outputs_count > 0 && process->audio_inputs_count > 0) {
        clap_audio_buffer_t *out_buf = &process->audio_outputs[0];
        const clap_audio_buffer_t *in_buf = &process->audio_inputs[0];
        
        if (out_buf->channel_count >= 1 && in_buf->channel_count >= 1 && 
            out_buf->data32 && in_buf->data32) {
            
            // Process each channel
            for (uint32_t ch = 0; ch < std::min(out_buf->channel_count, in_buf->channel_count); ++ch) {
                // Apply low-pass filter to the channel
                self->processor->ProcessAudio(
                    in_buf->data32[ch], 
                    out_buf->data32[ch], 
                    process->frames_count
                );
            }
        }
    }
    
    return CLAP_PROCESS_CONTINUE;
}

static const void *my_plugin_get_extension(const struct clap_plugin *plugin, const char *id) {
    printf("MyPlugin: Host requesting extension: %s\n", id);
    
    // Return GUI extension if requested
    if (strcmp(id, CLAP_EXT_GUI) == 0) {
        printf("MyPlugin: Returning GUI extension\n");
        return &my_plugin_gui_extension;
    }
    
    // Example: if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) return &my_audio_ports_extension;
    // Example: if (strcmp(id, CLAP_EXT_PARAMS) == 0) return &my_params_extension;
    return NULL; // Extension not supported
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

    // Store host pointer for GUI extension
    self->host = host;

    printf("MyPlugin: Plugin instance created successfully.\n");
    return &self->plugin;
}

const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory = {
    my_factory_get_plugin_count,
    my_factory_get_plugin_descriptor,
    my_factory_create_plugin,
};

// --- GUI Extension Function Implementations ---

static bool my_plugin_gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    printf("MyPlugin GUI: Checking API support for %s (floating: %s)\n", api, is_floating ? "true" : "false");
    
    // Support common windowing APIs
    if (strcmp(api, CLAP_WINDOW_API_X11) == 0) return true;
    if (strcmp(api, CLAP_WINDOW_API_WIN32) == 0) return true;
    if (strcmp(api, CLAP_WINDOW_API_COCOA) == 0) return true;
    if (strcmp(api, CLAP_WINDOW_API_WAYLAND) == 0) return is_floating; // Wayland only supports floating
    
    return false;
}

static bool my_plugin_gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating) {
    printf("MyPlugin GUI: Getting preferred API\n");
    
    // Prefer embedded windows over floating
    *is_floating = false;
    
    // Return platform-specific preferred API
#ifdef __linux__
    *api = CLAP_WINDOW_API_X11;
#elif defined(_WIN32)
    *api = CLAP_WINDOW_API_WIN32;
#elif defined(__APPLE__)
    *api = CLAP_WINDOW_API_COCOA;
#else
    *api = CLAP_WINDOW_API_X11; // Fallback
#endif
    
    return true;
}

static bool my_plugin_gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: Creating GUI with API %s (floating: %s)\n", api ? api : "null", is_floating ? "true" : "false");
    
    if (self->gui_created) {
        printf("MyPlugin GUI: GUI already created\n");
        return false;
    }
    
    try {
        // Initialize the Brisk UI
        if (self->ui_view && self->ui_view->Initialize(nullptr)) {
            self->gui_created = true;
            self->gui_visible = false;
            printf("MyPlugin GUI: GUI created successfully\n");
            return true;
        } else {
            printf("MyPlugin GUI: Failed to initialize Brisk UI\n");
            return false;
        }
    } catch (const std::exception& e) {
        printf("MyPlugin GUI: Exception during GUI creation: %s\n", e.what());
        return false;
    }
}

static void my_plugin_gui_destroy(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: Destroying GUI\n");
    
    if (self->gui_created) {
        self->gui_visible = false;
        self->gui_created = false;
        self->parent_window = nullptr;
        printf("MyPlugin GUI: GUI destroyed\n");
    }
}

static bool my_plugin_gui_set_scale(const clap_plugin_t *plugin, double scale) {
    printf("MyPlugin GUI: Setting scale to %f\n", scale);
    // TODO: Implement scaling support in Brisk UI
    return true; // Accept any scale for now
}

static bool my_plugin_gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    if (!self->gui_created) {
        printf("MyPlugin GUI: Cannot get size - GUI not created\n");
        return false;
    }
    
    *width = self->gui_width;
    *height = self->gui_height;
    printf("MyPlugin GUI: Returning size %ux%u\n", *width, *height);
    return true;
}

static bool my_plugin_gui_can_resize(const clap_plugin_t *plugin) {
    printf("MyPlugin GUI: Can resize - returning true\n");
    return true; // Allow resizing
}

static bool my_plugin_gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints) {
    printf("MyPlugin GUI: Getting resize hints\n");
    
    hints->can_resize_horizontally = true;
    hints->can_resize_vertically = true;
    hints->preserve_aspect_ratio = false;
    hints->aspect_ratio_width = 0;
    hints->aspect_ratio_height = 0;
    
    return true;
}

static bool my_plugin_gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    printf("MyPlugin GUI: Adjusting size from %ux%u\n", *width, *height);
    
    // Enforce minimum size
    if (*width < 200) *width = 200;
    if (*height < 150) *height = 150;
    
    // Enforce maximum size
    if (*width > 1200) *width = 1200;
    if (*height > 800) *height = 800;
    
    printf("MyPlugin GUI: Adjusted size to %ux%u\n", *width, *height);
    return true;
}

static bool my_plugin_gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: Setting size to %ux%u\n", width, height);
    
    if (!self->gui_created) {
        printf("MyPlugin GUI: Cannot set size - GUI not created\n");
        return false;
    }
    
    self->gui_width = width;
    self->gui_height = height;
    
    // TODO: Notify Brisk UI of size change
    return true;
}

static bool my_plugin_gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: Setting parent window (API: %s)\n", window->api);
    
    if (!self->gui_created) {
        printf("MyPlugin GUI: Cannot set parent - GUI not created\n");
        return false;
    }
    
    // Store parent window info
    self->parent_window = window->ptr;
    
    // Initialize Brisk UI with parent window
    try {
        if (self->ui_view && self->ui_view->Initialize(window->ptr)) {
            printf("MyPlugin GUI: Parent window set successfully\n");
            return true;
        } else {
            printf("MyPlugin GUI: Failed to set parent window\n");
            return false;
        }
    } catch (const std::exception& e) {
        printf("MyPlugin GUI: Exception setting parent window: %s\n", e.what());
        return false;
    }
}

static bool my_plugin_gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window) {
    printf("MyPlugin GUI: Setting transient window (API: %s)\n", window->api);
    // TODO: Implement transient window support for floating windows
    return true;
}

static void my_plugin_gui_suggest_title(const clap_plugin_t *plugin, const char *title) {
    printf("MyPlugin GUI: Suggested title: %s\n", title);
    // TODO: Set window title in Brisk UI
}

static bool my_plugin_gui_show(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: Showing GUI\n");
    
    if (!self->gui_created) {
        printf("MyPlugin GUI: Cannot show - GUI not created\n");
        return false;
    }
    
    try {
        if (self->ui_view) {
            self->ui_view->SetVisible(true);
            self->ui_view->UpdateUI();
            self->gui_visible = true;
            printf("MyPlugin GUI: GUI shown successfully\n");
            return true;
        } else {
            printf("MyPlugin GUI: No UI view available\n");
            return false;
        }
    } catch (const std::exception& e) {
        printf("MyPlugin GUI: Exception showing GUI: %s\n", e.what());
        return false;
    }
}

static bool my_plugin_gui_hide(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: Hiding GUI\n");
    
    if (!self->gui_created) {
        printf("MyPlugin GUI: Cannot hide - GUI not created\n");
        return false;
    }
    
    try {
        if (self->ui_view) {
            self->ui_view->SetVisible(false);
            self->gui_visible = false;
            printf("MyPlugin GUI: GUI hidden successfully\n");
            return true;
        } else {
            printf("MyPlugin GUI: No UI view available\n");
            return false;
        }
    } catch (const std::exception& e) {
        printf("MyPlugin GUI: Exception hiding GUI: %s\n", e.what());
        return false;
    }
}

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
