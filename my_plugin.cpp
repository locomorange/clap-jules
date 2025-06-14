#include "my_plugin.h"
#if VSTGUI_ENABLED
#include "my_plugin_gui.h"
#include <clap/ext/gui.h>
#endif
#include <clap/plugin-features.h>
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc

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


// --- Plugin Implementation ---
static bool my_plugin_init(const struct clap_plugin *plugin) {
    // my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Initializing plugin\n");
    // Initialize your plugin state here
    return true;
}

static void my_plugin_destroy(const struct clap_plugin *plugin) {
    printf("MyPlugin: Destroying plugin\n");
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self) {
#if VSTGUI_ENABLED
        if (self->gui_editor) {
            delete self->gui_editor;
            self->gui_editor = nullptr;
        }
#endif
        free(self);
    }
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
    // This is where the main audio processing happens.
    // For this example, we'll just print a message once.
    // static bool first_process = true;
    // if (first_process) {
    //     printf("MyPlugin: Processing audio...\n");
    //     first_process = false;
    // }

    // Example: Iterate over input events
    // const uint32_t num_events = process->in_events->size(process->in_events);
    // for (uint32_t i = 0; i < num_events; ++i) {
    //     const clap_event_header_t* hdr = process->in_events->get(process->in_events, i);
    //     if (hdr->space_id == CLAP_CORE_EVENT_SPACE_ID) {
    //         switch (hdr->type) {
    //             case CLAP_EVENT_NOTE_ON:
    //                 // const clap_event_note_t* nev = (const clap_event_note_t*)hdr;
    //                 // Handle note on
    //                 break;
    //             case CLAP_EVENT_NOTE_OFF:
    //                 // const clap_event_note_t* nev = (const clap_event_note_t*)hdr;
    //                 // Handle note off
    //                 break;
    //             // Add other event types as needed
    //         }
    //     }
    // }

    // Example: Process audio from input to output (stereo)
    // if (process->audio_outputs_count > 0 && process->audio_inputs_count > 0) {
    //     clap_audio_buffer_t *out_buf = &process->audio_outputs[0];
    //     clap_audio_buffer_t *in_buf = &process->audio_inputs[0];
    //
    //     if (out_buf->channel_count >= 2 && in_buf->channel_count >=2 && out_buf->data32 && in_buf->data32) {
    //         for (uint32_t i = 0; i < process->frames_count; ++i) {
    //             out_buf->data32[0][i] = in_buf->data32[0][i]; // Left channel
    //             out_buf->data32[1][i] = in_buf->data32[1][i]; // Right channel
    //         }
    //     }
    // }
    return CLAP_PROCESS_CONTINUE;
}

// --- GUI Extension Implementation ---
#if VSTGUI_ENABLED
static bool my_plugin_gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->isApiSupported(api, is_floating);
    }
    return false;
}

static bool my_plugin_gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->getPreferredApi(api, is_floating);
    }
    return false;
}

static bool my_plugin_gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->create(api, is_floating);
    }
    return false;
}

static void my_plugin_gui_destroy(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        self->gui_editor->destroy();
    }
}

static bool my_plugin_gui_set_scale(const clap_plugin_t *plugin, double scale) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->setScale(scale);
    }
    return false;
}

static bool my_plugin_gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->getSize(width, height);
    }
    return false;
}

static bool my_plugin_gui_can_resize(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->canResize();
    }
    return false;
}

static bool my_plugin_gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->getResizeHints(hints);
    }
    return false;
}

static bool my_plugin_gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->adjustSize(width, height);
    }
    return false;
}

static bool my_plugin_gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->setSize(width, height);
    }
    return false;
}

static bool my_plugin_gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->setParent(window);
    }
    return false;
}

static bool my_plugin_gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->setTransient(window);
    }
    return false;
}

static void my_plugin_gui_suggest_title(const clap_plugin_t *plugin, const char *title) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        self->gui_editor->suggestTitle(title);
    }
}

static bool my_plugin_gui_show(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->show();
    }
    return false;
}

static bool my_plugin_gui_hide(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self && self->gui_editor) {
        return self->gui_editor->hide();
    }
    return false;
}

static const clap_plugin_gui_t my_gui_extension = {
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
    my_plugin_gui_hide
};
#endif // VSTGUI_ENABLED

static const void *my_plugin_get_extension(const struct clap_plugin *plugin, const char *id) {
    // Example: if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) return &my_audio_ports_extension;
    // Example: if (strcmp(id, CLAP_EXT_PARAMS) == 0) return &my_params_extension;
    printf("MyPlugin: Host requesting extension: %s\n", id);
    
#if VSTGUI_ENABLED
    if (strcmp(id, CLAP_EXT_GUI) == 0) {
        return &my_gui_extension;
    }
#endif
    
    return NULL; // No other extensions supported in this basic example
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

    // Initialize GUI editor
#if VSTGUI_ENABLED
    self->gui_editor = new MyPluginEditor();
    if (!self->gui_editor) {
        fprintf(stderr, "MyPlugin: Error - failed to create GUI editor\n");
        free(self);
        return NULL;
    }
#endif

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
