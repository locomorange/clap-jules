#include "my_plugin.h"
#include "src/plugin_viewmodel.h"
#include "src/audio_processor.h"
#include "src/brisk_clap_integration.h" // Ensure brisk_clap::BriskClapGUI is available
#include "src/cross_platform_gui_support.h" // Cross-platform GUI support
#include <clap/ext/gui.h>
#include <clap/ext/params.h>
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc
#include <algorithm> // For std::min
#include <exception> // For exception handling
#include <brisk/gui/Component.hpp>
#include <brisk/gui/GuiApplication.hpp>
#include <brisk/widgets/Graphene.hpp>
#include <brisk/widgets/Button.hpp>
#include <brisk/widgets/Layouts.hpp>
#include <brisk/widgets/Text.hpp>

//Brisk::GuiApplication application;
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

// --- Parameters Extension Forward Declarations ---
static uint32_t my_plugin_params_count(const clap_plugin_t *plugin);
static bool my_plugin_params_get_info(const clap_plugin_t *plugin, uint32_t param_index, clap_param_info_t *param_info);
static bool my_plugin_params_get_value(const clap_plugin_t *plugin, clap_id param_id, double *value);
static bool my_plugin_params_value_to_text(const clap_plugin_t *plugin, clap_id param_id, double value, char *display, uint32_t size);
static bool my_plugin_params_text_to_value(const clap_plugin_t *plugin, clap_id param_id, const char *display, double *value);
static void my_plugin_params_flush(const clap_plugin_t *plugin, const clap_input_events_t *in, const clap_output_events_t *out);

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

// --- Parameters Extension Implementation ---
static const clap_plugin_params_t my_plugin_params_extension = {
    my_plugin_params_count,
    my_plugin_params_get_info,
    my_plugin_params_get_value,
    my_plugin_params_value_to_text,
    my_plugin_params_text_to_value,
    my_plugin_params_flush,
};

// Parameter definitions
#define PARAM_CUTOFF_FREQUENCY 0

static const clap_param_info_t param_info_cutoff = {
    PARAM_CUTOFF_FREQUENCY,  // id
    CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_MODULATABLE,  // flags
    nullptr,  // cookie
    "Cutoff Frequency",  // name
    "",  // module
    20.0,  // min_value (20 Hz)
    20000.0,  // max_value (20 kHz)
    1000.0,  // default_value (1 kHz)
};


// --- Plugin Implementation ---
static bool my_plugin_init(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Initializing plugin\n");
    
    try {
        // Initialize MVVM components in correct dependency order
        self->model = std::make_shared<plugin::PluginModel>();
        self->processor = std::make_shared<plugin::AudioProcessor>(self->model);
        self->viewmodel = std::make_shared<plugin::PluginViewModel>(self->model, self->processor);
        self->ui_view = std::make_shared<brisk_clap::BriskClapGUI>(); // Now brisk_clap should be available
        
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
    
    // Process parameter events
    if (process->in_events) {
        for (uint32_t i = 0; i < process->in_events->size(process->in_events); ++i) {
            const clap_event_header_t *event = process->in_events->get(process->in_events, i);
            if (event->type == CLAP_EVENT_PARAM_VALUE && event->space_id == CLAP_CORE_EVENT_SPACE_ID) {
                const clap_event_param_value_t *param_event = (const clap_event_param_value_t *)event;
                if (param_event->param_id == PARAM_CUTOFF_FREQUENCY && self->viewmodel) {
                    self->viewmodel->OnCutoffFrequencyChanged(param_event->value);
                }
            }
        }
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
    
    // Return parameters extension if requested
    if (strcmp(id, CLAP_EXT_PARAMS) == 0) {
        printf("MyPlugin: Returning parameters extension\n");
        return &my_plugin_params_extension;
    }
    
    // Example: if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) return &my_audio_ports_extension;
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
    
    // Use cross-platform helper to check API support
    return brisk_clap::CrossPlatformGUISupport::isApiSupportedOnPlatform(api, is_floating);
}

static bool my_plugin_gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating) {
    printf("MyPlugin GUI: Getting preferred API\n");
    
    // Get preferred API using cross-platform helper
    *api = brisk_clap::CrossPlatformGUISupport::getPreferredApi();
    
    // Prefer embedded windows unless the API requires floating
    if (*api) {
        *is_floating = brisk_clap::CrossPlatformGUISupport::prefersFloatingWindows(*api);
        return true;
    }
    
    return false;
}

static bool my_plugin_gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: Creating GUI with API %s (floating: %s)\n", api ? api : "null", is_floating ? "true" : "false");
    
    if (self->gui_created) {
        printf("MyPlugin GUI: GUI already created\n");
        return false;
    }
    
    try {
        // Initialize the BriskClapGUI
        if (self->ui_view && self->ui_view->initialize()) {
            self->gui_created = true;
            self->gui_visible = false;
            printf("MyPlugin GUI: GUI created successfully\n");
            return true;
        } else {
            printf("MyPlugin GUI: Failed to initialize BriskClapGUI\n");
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
        if (self->ui_view) {
            self->ui_view->destroyWindow();
        }
        self->gui_visible = false;
        self->gui_created = false;
        self->parent_window = nullptr;
        printf("MyPlugin GUI: GUI destroyed\n");
    }
}

static bool my_plugin_gui_set_scale(const clap_plugin_t *plugin, double scale) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: Setting scale to %f\n", scale);
    
    if (!self->gui_created || !self->ui_view) {
        printf("MyPlugin GUI: Cannot set scale - GUI not created\n");
        return false;
    }
    
    // Apply scaling to Brisk GUI
    if (self->ui_view->setScale(scale)) {
        printf("MyPlugin GUI: Scale applied successfully\n");
        return true;
    } else {
        printf("MyPlugin GUI: Failed to apply scale\n");
        return false;
    }
}

static bool my_plugin_gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    if (!self->gui_created) {
        printf("MyPlugin GUI: Cannot get size - GUI not created\n");
        return false;
    }
    
    if (self->ui_view) {
        self->ui_view->getSize(width, height);
        printf("MyPlugin GUI: Returning size %ux%u\n", *width, *height);
        return true;
    }
    
    // Fallback to stored values
    *width = self->gui_width;
    *height = self->gui_height;
    printf("MyPlugin GUI: Returning fallback size %ux%u\n", *width, *height);
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
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: Adjusting size from %ux%u\n", *width, *height);
    
    // Use cross-platform validation if we have a known API
    const char* api = nullptr;
    if (self->ui_view) {
        // Get the API from the UI view if available
        api = brisk_clap::CrossPlatformGUISupport::getPreferredApi();
    }
    
    if (api && brisk_clap::CrossPlatformGUISupport::validateSize(api, width, height)) {
        printf("MyPlugin GUI: Platform-validated size to %ux%u\n", *width, *height);
    } else {
        // Fallback to manual constraints
        if (*width < 200) *width = 200;
        if (*height < 150) *height = 150;
        if (*width > 1200) *width = 1200;
        if (*height > 800) *height = 800;
        printf("MyPlugin GUI: Manually adjusted size to %ux%u\n", *width, *height);
    }
    
    // Also use the Brisk GUI's adjust size if available
    if (self->ui_view && self->gui_created) {
        return self->ui_view->adjustSize(width, height);
    }
    
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
    
    // Update MinimalBriskGUI size
    if (self->ui_view) {
        return self->ui_view->setSize(width, height);
    }
    
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
    
    // Create the Brisk window with the parent
    if (self->ui_view) {
        return self->ui_view->createWindow(window);
    }
    printf("MyPlugin GUI: Parent window set successfully\n");
    return true;
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
            self->ui_view->show();
            // Start processing events
            self->ui_view->processEvents();
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
            self->ui_view->hide();
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

// --- Parameters Extension Function Implementations ---

static uint32_t my_plugin_params_count(const clap_plugin_t *plugin) {
    printf("MyPlugin Params: Returning parameter count: 1\n");
    return 1; // We have one parameter: cutoff frequency
}

static bool my_plugin_params_get_info(const clap_plugin_t *plugin, uint32_t param_index, clap_param_info_t *param_info) {
    printf("MyPlugin Params: Getting parameter info for index %u\n", param_index);
    
    if (param_index == 0) {
        *param_info = param_info_cutoff;
        return true;
    }
    
    return false;
}

static bool my_plugin_params_get_value(const clap_plugin_t *plugin, clap_id param_id, double *value) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin Params: Getting value for parameter %u\n", param_id);
    
    if (param_id == PARAM_CUTOFF_FREQUENCY && self->model) {
        *value = self->model->GetCutoffFrequency();
        return true;
    }
    
    return false;
}

static bool my_plugin_params_value_to_text(const clap_plugin_t *plugin, clap_id param_id, double value, char *display, uint32_t size) {
    printf("MyPlugin Params: Converting value %.2f to text for parameter %u\n", value, param_id);
    
    if (param_id == PARAM_CUTOFF_FREQUENCY) {
        snprintf(display, size, "%.1f Hz", value);
        return true;
    }
    
    return false;
}

static bool my_plugin_params_text_to_value(const clap_plugin_t *plugin, clap_id param_id, const char *display, double *value) {
    printf("MyPlugin Params: Converting text '%s' to value for parameter %u\n", display, param_id);
    
    if (param_id == PARAM_CUTOFF_FREQUENCY) {
        char *endptr;
        double parsed_value = strtod(display, &endptr);
        if (endptr != display) {
            *value = std::max(20.0, std::min(20000.0, parsed_value));
            return true;
        }
    }
    
    return false;
}

static void my_plugin_params_flush(const clap_plugin_t *plugin, const clap_input_events_t *in, const clap_output_events_t *out) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin Params: Flushing parameters (processing %u input events)\n", in->size(in));
    
    // Process parameter change events
    for (uint32_t i = 0; i < in->size(in); ++i) {
        const clap_event_header_t *event = in->get(in, i);
        if (event->type == CLAP_EVENT_PARAM_VALUE && event->space_id == CLAP_CORE_EVENT_SPACE_ID) {
            const clap_event_param_value_t *param_event = (const clap_event_param_value_t *)event;
            printf("MyPlugin Params: Parameter %u changed to %f\n", param_event->param_id, param_event->value);
            
            if (param_event->param_id == PARAM_CUTOFF_FREQUENCY && self->viewmodel) {
                self->viewmodel->OnCutoffFrequencyChanged(param_event->value);
                
                // Update UI if available
                if (self->ui_view) {
                    self->ui_view->updateParameter(param_event->param_id, param_event->value);
                }
            }
        }
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
