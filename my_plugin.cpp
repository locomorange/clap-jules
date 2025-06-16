#include "my_plugin.h"
#include "graphics_renderer.h"
#include <cstring>   // For strcmp
#include <cstdlib>   // For calloc
#include <memory>    // For unique_ptr

// --- Forward declarations of plugin functions ---
bool CLAP_ABI my_plugin_init(const struct clap_plugin *plugin);
void CLAP_ABI my_plugin_destroy(const struct clap_plugin *plugin);
bool CLAP_ABI my_plugin_activate(const struct clap_plugin *plugin, double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count);
void CLAP_ABI my_plugin_deactivate(const struct clap_plugin *plugin);
bool CLAP_ABI my_plugin_start_processing(const struct clap_plugin *plugin);
void CLAP_ABI my_plugin_stop_processing(const struct clap_plugin *plugin);
void CLAP_ABI my_plugin_reset(const struct clap_plugin *plugin);
clap_process_status CLAP_ABI my_plugin_process(const struct clap_plugin *plugin, const clap_process_t *process);
const void * CLAP_ABI my_plugin_get_extension(const struct clap_plugin *plugin, const char *id);
void CLAP_ABI my_plugin_on_main_thread(const struct clap_plugin *plugin);

// --- Plugin Descriptor ---
// Features array for the plugin descriptor
static const char *const plugin_features[] = {"audio_effect", nullptr};

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
bool CLAP_ABI my_plugin_init(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    // Initialize graphics renderer
    clap_jules::initializeGraphics(self);
    
    return true;
}

void CLAP_ABI my_plugin_destroy(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    // Cleanup graphics renderer
    clap_jules::cleanupGraphics(self);
    
    // Free any resources allocated in init
}

bool CLAP_ABI my_plugin_activate(const struct clap_plugin *plugin, double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count) {
    (void)sample_rate; (void)min_frames_count; (void)max_frames_count; // Suppress unused parameter warnings
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    // Demonstrate graphics rendering when plugin is activated
    clap_jules::renderFrame(self, 320, 240);
    
    // Allocate and prepare resources needed for processing (e.g., buffers)
    return true;
}

void CLAP_ABI my_plugin_deactivate(const struct clap_plugin *plugin) {
    (void)plugin; // Suppress unused parameter warning
    // Free resources allocated in activate
}

bool CLAP_ABI my_plugin_start_processing(const struct clap_plugin *plugin) {
    (void)plugin; // Suppress unused parameter warning
    return true;
}

void CLAP_ABI my_plugin_stop_processing(const struct clap_plugin *plugin) {
    (void)plugin; // Suppress unused parameter warning
}

void CLAP_ABI my_plugin_reset(const struct clap_plugin *plugin) {
    (void)plugin; // Suppress unused parameter warning
    // Reset plugin state (e.g., clear buffers, reset parameters)
}

clap_process_status CLAP_ABI my_plugin_process(const struct clap_plugin *plugin, const clap_process_t *process) {
    (void)plugin; (void)process; // Suppress unused parameter warnings
    // This is where the main audio processing happens.
    // For this example, we'll just print a message once.
    // static bool first_process = true;
    // if (first_process) {
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

const void * CLAP_ABI my_plugin_get_extension(const struct clap_plugin *plugin, const char *id) {
    (void)plugin; (void)id; // Suppress unused parameter warnings
    // Example: if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) return &my_audio_ports_extension;
    // Example: if (strcmp(id, CLAP_EXT_PARAMS) == 0) return &my_params_extension;
    return NULL; // No extensions supported in this basic example
}

void CLAP_ABI my_plugin_on_main_thread(const struct clap_plugin *plugin) {
    (void)plugin; // Suppress unused parameter warning
    // Called by the host to perform tasks that must run on the main thread.
}

// --- Plugin Entry Point (clap_plugin_entry) ---
// This is not directly part of the clap_plugin_t struct but is essential.
// It's usually defined in the factory.

// --- Plugin Factory ---
// This structure is responsible for creating plugin instances.

uint32_t CLAP_ABI my_factory_get_plugin_count(const struct clap_plugin_factory *factory) {
    (void)factory; // Suppress unused parameter warning
    return 1; // We have one plugin in this factory
}

const clap_plugin_descriptor_t * CLAP_ABI my_factory_get_plugin_descriptor(const struct clap_plugin_factory *factory, uint32_t index) {
    (void)factory; // Suppress unused parameter warning
    if (index == 0) {
        return &my_plugin_descriptor;
    }
    return NULL;
}

const clap_plugin_t * CLAP_ABI my_factory_create_plugin(const struct clap_plugin_factory *factory, const clap_host_t *host, const char *plugin_id) {
    (void)factory; (void)host; // Suppress unused parameter warnings
    if (strcmp(plugin_id, my_plugin_descriptor.id) != 0) {
        return NULL;
    }

    my_plugin_t *self = (my_plugin_t *)calloc(1, sizeof(my_plugin_t));
    if (!self) {
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

    return &self->plugin;
}

extern "C" {

const struct clap_plugin_factory my_plugin_factory = {
    my_factory_get_plugin_count,
    my_factory_get_plugin_descriptor,
    my_factory_create_plugin,
};

// --- CLAP Entry Point Functions ---
// Entry point functions with proper CLAP_ABI calling convention for Windows compatibility
bool CLAP_ABI plugin_entry_init(const char *plugin_path) {
    (void)plugin_path; // Suppress unused parameter warning
    // Perform any global library initialization here if needed
    return true;
}

void CLAP_ABI plugin_entry_deinit(void) {
    // Perform any global library cleanup here if needed
}

const void * CLAP_ABI plugin_entry_get_factory(const char *factory_id) {
    if (strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID) == 0) {
        return &my_plugin_factory;
    }
    // To support other factory types, check their specific IDs here.
    // For example, CLAP_PLUGIN_VOICE_INFO_FACTORY_ID for voice info.
    // Or CLAP_PLUGIN_REMOTABLE_CONTROLS_FACTORY_ID for remotable controls.
    return NULL;
}

} // extern "C"

// --- CLAP Entry Point ---
// This is the main entry point that the host will look for.
// Must follow CLAP specification exactly for proper export on all platforms
extern "C" CLAP_EXPORT const clap_plugin_entry_t clap_entry = {
    CLAP_VERSION,
    plugin_entry_init,
    plugin_entry_deinit,
    plugin_entry_get_factory
};

// --- C++ Graphics Implementation ---
namespace clap_jules {

void initializeGraphics(my_plugin_t* plugin) {
    try {
        // Initialize graphics renderer
        auto renderer = createGraphicsRenderer();
        plugin->graphics_renderer = renderer.release();
    } catch (...) {
        // Ensure no exceptions escape to C code
        plugin->graphics_renderer = nullptr;
    }
}

void cleanupGraphics(my_plugin_t* plugin) {
    try {
        if (plugin->graphics_renderer) {
            delete static_cast<GraphicsRenderer*>(plugin->graphics_renderer);
            plugin->graphics_renderer = nullptr;
        }
    } catch (...) {
        // Ensure no exceptions escape to C code
        plugin->graphics_renderer = nullptr;
    }
}

void renderFrame(my_plugin_t* plugin, int width, int height) {
    try {
        if (!plugin->graphics_renderer) return;
        
        auto* renderer = static_cast<GraphicsRenderer*>(plugin->graphics_renderer);
        
        // Demonstrate graphics rendering
        renderer->beginFrame(width, height);
        
        // Clear background
        renderer->clear(0xFF222222); // Dark gray background
        
        // Draw some example graphics
        renderer->drawRect(10, 10, 100, 50, 0xFF4CAF50); // Green rectangle
        renderer->drawCircle(200, 50, 30, 0xFF2196F3);   // Blue circle
        renderer->drawText("CLAP + Graphics", 10, 80, 0xFFFFFFFF); // White text
        
        // Draw a simple spectrum visualization placeholder
        for (int i = 0; i < 10; ++i) {
            float barHeight = 20 + (i * 5); // Simulated spectrum data
            renderer->drawRect(50 + i * 15, 120 - barHeight, 10, barHeight, 0xFFFF9800); // Orange bars
        }
        
        renderer->endFrame();
    } catch (...) {
        // Ensure no exceptions escape to C code - silently handle any errors
    }
}

} // namespace clap_jules
