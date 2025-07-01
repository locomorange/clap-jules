#include "my_plugin.h"
#include "src/AudioProcessor.h"
#include "src/MVVM.h"
#include "src/DIContainer.h"
#include "src/ClapParameterExtension.h"
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc
#include <memory>
#include <new>

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
static const char *const plugin_features[] = {"audio_effect", nullptr};

static const clap_plugin_descriptor_t my_plugin_descriptor = {
    CLAP_VERSION,
    "com.clapejules.filterPlugin", // id
    "ClapeJules Filter Plugin", // name
    "ClapeJules",           // vendor
    "https://github.com/locomorange/clap-jules",  // url
    "https://github.com/locomorange/clap-jules/issues", // manual_url
    "https://github.com/locomorange/clap-jules",    // support_url
    "0.1.0",                // version
    "CLAP plugin with KFR lowpass filter and Brisk MVVM UI.", // description
    plugin_features, // features
};


// --- Plugin Implementation ---
static bool my_plugin_init(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("ClapeJules: Initializing plugin with DI container\n");
    
    try {
        // Create DI container and inject dependencies
        auto container = ClapeJules::DI::createContainer();
        
        // Create components using dependency injection
        self->audioProcessor = container.create<std::shared_ptr<ClapeJules::AudioProcessor>>();
        self->viewModel = container.create<std::shared_ptr<ClapeJules::FilterViewModel>>();
        self->view = container.create<std::shared_ptr<ClapeJules::FilterView>>();
        
        // Store container (note: we need to be careful about this since container type is complex)
        // For now, we'll just rely on the shared_ptrs keeping everything alive
        
        printf("ClapeJules: Successfully initialized MVVM components\n");
        return true;
    } catch (const std::exception& e) {
        printf("ClapeJules: Failed to initialize - %s\n", e.what());
        return false;
    }
}

static void my_plugin_destroy(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("ClapeJules: Destroying plugin\n");
    
    // Reset shared_ptrs to clean up MVVM components
    self->view.reset();
    self->viewModel.reset(); 
    self->audioProcessor.reset();
}

static bool my_plugin_activate(const struct clap_plugin *plugin, double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("ClapeJules: Activating plugin (Sample Rate: %.2f, Min Frames: %u, Max Frames: %u)\n", sample_rate, min_frames_count, max_frames_count);
    
    if (self->audioProcessor) {
        self->audioProcessor->setSampleRate(sample_rate);
        printf("ClapeJules: Set sample rate to %.2f Hz\n", sample_rate);
    }
    
    return true;
}

static void my_plugin_deactivate(const struct clap_plugin *plugin) {
    printf("ClapeJules: Deactivating plugin\n");
    // Clean up any processing-specific resources
}

static bool my_plugin_start_processing(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("ClapeJules: Starting processing\n");
    
    if (self->audioProcessor) {
        self->audioProcessor->reset();
    }
    
    return true;
}

static void my_plugin_stop_processing(const struct clap_plugin *plugin) {
    printf("ClapeJules: Stopping processing\n");
}

static void my_plugin_reset(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("ClapeJules: Resetting plugin\n");
    
    if (self->audioProcessor) {
        self->audioProcessor->reset();
    }
}

static clap_process_status my_plugin_process(const struct clap_plugin *plugin, const clap_process_t *process) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    // Process audio using the KFR lowpass filter
    if (self->audioProcessor && process->audio_outputs_count > 0 && process->audio_inputs_count > 0) {
        clap_audio_buffer_t *out_buf = &process->audio_outputs[0];
        const clap_audio_buffer_t *in_buf = &process->audio_inputs[0];

        if (out_buf->data32 && in_buf->data32 && process->frames_count > 0) {
            // Apply the lowpass filter using our AudioProcessor
            self->audioProcessor->process(
                in_buf->data32, 
                out_buf->data32, 
                process->frames_count, 
                std::min(in_buf->channel_count, out_buf->channel_count)
            );
        }
    }
    
    return CLAP_PROCESS_CONTINUE;
}

static const void *my_plugin_get_extension(const struct clap_plugin *plugin, const char *id) {
    if (strcmp(id, CLAP_EXT_PARAMS) == 0) {
        return ClapeJules::ClapExt::ParameterExtension::getInterface();
    }
    
    printf("ClapeJules: Host requesting extension: %s\n", id);
    return NULL; // Extension not supported
}

static void my_plugin_on_main_thread(const struct clap_plugin *plugin) {
    // Called by the host to perform tasks that must run on the main thread.
    // printf("ClapeJules: on_main_thread called\n");
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
