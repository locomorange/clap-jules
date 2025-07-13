#include "my_plugin.h"
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc
#include <cmath>    // For pow function

// Helper function to convert dB to linear scale
static double db_to_linear(double db) {
    return pow(10.0, db / 20.0);
}

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

// --- Parameter extension functions ---
static uint32_t my_plugin_params_count(const clap_plugin_t *plugin);
static bool my_plugin_params_get_info(const clap_plugin_t *plugin, uint32_t param_index, clap_param_info_t *param_info);
static bool my_plugin_params_get_value(const clap_plugin_t *plugin, clap_id param_id, double *value);
static bool my_plugin_params_value_to_text(const clap_plugin_t *plugin, clap_id param_id, double value, char *display, uint32_t size);
static bool my_plugin_params_text_to_value(const clap_plugin_t *plugin, clap_id param_id, const char *display, double *value);
static void my_plugin_params_flush(const clap_plugin_t *plugin, const clap_input_events_t *in, const clap_output_events_t *out);

// --- Audio ports extension functions ---
static uint32_t my_plugin_audio_ports_count(const clap_plugin_t *plugin, bool is_input);
static bool my_plugin_audio_ports_get(const clap_plugin_t *plugin, uint32_t index, bool is_input, clap_audio_port_info_t *info);

// --- Plugin Descriptor ---
// Features array for the plugin descriptor
static const char *const plugin_features[] = {"audio-effect", nullptr};

// --- Extension implementations ---
static const clap_plugin_params_t my_plugin_params_extension = {
    .count = my_plugin_params_count,
    .get_info = my_plugin_params_get_info,
    .get_value = my_plugin_params_get_value,
    .value_to_text = my_plugin_params_value_to_text,
    .text_to_value = my_plugin_params_text_to_value,
    .flush = my_plugin_params_flush,
};

static const clap_plugin_audio_ports_t my_plugin_audio_ports_extension = {
    .count = my_plugin_audio_ports_count,
    .get = my_plugin_audio_ports_get,
};

static const clap_plugin_descriptor_t my_plugin_descriptor = {
    CLAP_VERSION,
    "com.example.gainfx", // id
    "Gain Control FX", // name
    "My Company",           // vendor
    "https://example.com",  // url
    "https://example.com/bugtracker", // manual_url
    "https://example.com/support",    // support_url
    "0.1.0",                // version
    "Simple gain control plugin with -36dB to +36dB range.", // description
    plugin_features, // features
};


// --- Plugin Implementation ---
static bool my_plugin_init(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Initializing plugin\n");
    
    // Initialize gain to 0dB (no change)
    self->gain_db = 0.0;
    self->gain_linear = 1.0;
    self->sample_rate = 44100.0; // Default, will be updated in activate
    
    return true;
}

static void my_plugin_destroy(const struct clap_plugin *plugin) {
    printf("MyPlugin: Destroying plugin\n");
    // Free any resources allocated in init
}

static bool my_plugin_activate(const struct clap_plugin *plugin, double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Activating plugin (Sample Rate: %.2f, Min Frames: %u, Max Frames: %u)\n", sample_rate, min_frames_count, max_frames_count);
    
    // Store sample rate
    self->sample_rate = sample_rate;
    
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
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    // Process parameter changes
    const uint32_t num_events = process->in_events->size(process->in_events);
    for (uint32_t i = 0; i < num_events; ++i) {
        const clap_event_header_t* hdr = process->in_events->get(process->in_events, i);
        if (hdr->space_id == CLAP_CORE_EVENT_SPACE_ID) {
            switch (hdr->type) {
                case CLAP_EVENT_PARAM_VALUE: {
                    const clap_event_param_value_t* param_event = (const clap_event_param_value_t*)hdr;
                    if (param_event->param_id == PARAM_GAIN_ID) {
                        self->gain_db = param_event->value;
                        self->gain_linear = db_to_linear(self->gain_db);
                    }
                    break;
                }
            }
        }
    }

    // Process audio from input to output (stereo)
    if (process->audio_outputs_count > 0 && process->audio_inputs_count > 0) {
        clap_audio_buffer_t *out_buf = &process->audio_outputs[0];
        const clap_audio_buffer_t *in_buf = &process->audio_inputs[0];

        if (out_buf->channel_count >= 2 && in_buf->channel_count >= 2 && out_buf->data32 && in_buf->data32) {
            for (uint32_t i = 0; i < process->frames_count; ++i) {
                out_buf->data32[0][i] = in_buf->data32[0][i] * (float)self->gain_linear; // Left channel
                out_buf->data32[1][i] = in_buf->data32[1][i] * (float)self->gain_linear; // Right channel
            }
        }
    }
    
    return CLAP_PROCESS_CONTINUE;
}

static const void *my_plugin_get_extension(const struct clap_plugin *plugin, const char *id) {
    printf("MyPlugin: Host requesting extension: %s\n", id);
    
    if (strcmp(id, CLAP_EXT_PARAMS) == 0) {
        return &my_plugin_params_extension;
    }
    if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) {
        return &my_plugin_audio_ports_extension;
    }
    
    return NULL; // Extension not supported
}

static void my_plugin_on_main_thread(const struct clap_plugin *plugin) {
    // Called by the host to perform tasks that must run on the main thread.
    // printf("MyPlugin: on_main_thread called\n");
}

// --- Parameter Extension Implementation ---
static uint32_t my_plugin_params_count(const clap_plugin_t *plugin) {
    return 1; // We have one parameter: gain
}

static bool my_plugin_params_get_info(const clap_plugin_t *plugin, uint32_t param_index, clap_param_info_t *param_info) {
    if (param_index == 0) {
        param_info->id = PARAM_GAIN_ID;
        param_info->flags = CLAP_PARAM_IS_AUTOMATABLE;
        param_info->cookie = nullptr;
        strcpy(param_info->name, "Gain");
        strcpy(param_info->module, "");
        param_info->min_value = -36.0;
        param_info->max_value = 36.0;
        param_info->default_value = 0.0;
        return true;
    }
    return false;
}

static bool my_plugin_params_get_value(const clap_plugin_t *plugin, clap_id param_id, double *value) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (param_id == PARAM_GAIN_ID) {
        *value = self->gain_db;
        return true;
    }
    return false;
}

static bool my_plugin_params_value_to_text(const clap_plugin_t *plugin, clap_id param_id, double value, char *display, uint32_t size) {
    if (param_id == PARAM_GAIN_ID) {
        snprintf(display, size, "%.1f dB", value);
        return true;
    }
    return false;
}

static bool my_plugin_params_text_to_value(const clap_plugin_t *plugin, clap_id param_id, const char *display, double *value) {
    if (param_id == PARAM_GAIN_ID) {
        double parsed_value;
        if (sscanf(display, "%lf", &parsed_value) == 1) {
            // Clamp to valid range
            if (parsed_value < -36.0) parsed_value = -36.0;
            if (parsed_value > 36.0) parsed_value = 36.0;
            *value = parsed_value;
            return true;
        }
    }
    return false;
}

static void my_plugin_params_flush(const clap_plugin_t *plugin, const clap_input_events_t *in, const clap_output_events_t *out) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    // Process parameter changes
    const uint32_t num_events = in->size(in);
    for (uint32_t i = 0; i < num_events; ++i) {
        const clap_event_header_t* hdr = in->get(in, i);
        if (hdr->space_id == CLAP_CORE_EVENT_SPACE_ID) {
            switch (hdr->type) {
                case CLAP_EVENT_PARAM_VALUE: {
                    const clap_event_param_value_t* param_event = (const clap_event_param_value_t*)hdr;
                    if (param_event->param_id == PARAM_GAIN_ID) {
                        self->gain_db = param_event->value;
                        self->gain_linear = db_to_linear(self->gain_db);
                    }
                    break;
                }
            }
        }
    }
}

// --- Audio Ports Extension Implementation ---
static uint32_t my_plugin_audio_ports_count(const clap_plugin_t *plugin, bool is_input) {
    return 1; // One stereo input and one stereo output
}

static bool my_plugin_audio_ports_get(const clap_plugin_t *plugin, uint32_t index, bool is_input, clap_audio_port_info_t *info) {
    if (index == 0) {
        info->id = is_input ? 0 : 1;
        strcpy(info->name, is_input ? "Audio In" : "Audio Out");
        info->flags = CLAP_AUDIO_PORT_IS_MAIN;
        info->channel_count = 2;
        info->port_type = CLAP_PORT_STEREO;
        info->in_place_pair = is_input ? 1 : 0; // Input port 0 pairs with output port 1
        return true;
    }
    return false;
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
