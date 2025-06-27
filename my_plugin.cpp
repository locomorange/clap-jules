#include "my_plugin.h"
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc

#include "plugin/service_configuration.hpp"
#include "plugin/plugin_view_model.hpp"
#include "plugin/plugin_window.hpp"
#include "audio/audio_processor.hpp"

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
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Initializing plugin\n");
    
    try {
        // Configure dependency injection
        plugin::ServiceConfiguration::configure();
        
        // Create view model and window
        self->viewModel = plugin::ServiceConfiguration::createViewModel();
        self->window = plugin::ServiceConfiguration::createWindow(self->viewModel);
        
        // Initialize plugin state
        self->sampleRate = 44100.0;
        self->bufferSize = 512;
        self->isProcessing = false;
        
        printf("MyPlugin: MVVM architecture initialized successfully\n");
        return true;
    } catch (const std::exception& e) {
        printf("MyPlugin: Failed to initialize MVVM architecture: %s\n", e.what());
        return false;
    }
}

static void my_plugin_destroy(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Destroying plugin\n");
    
    try {
        // Clean up MVVM components
        if (self->window) {
            self->window->hide();
            self->window.reset();
        }
        
        if (self->viewModel) {
            self->viewModel.reset();
        }
        
        printf("MyPlugin: MVVM components cleaned up successfully\n");
    } catch (const std::exception& e) {
        printf("MyPlugin: Error during cleanup: %s\n", e.what());
    }
}

static bool my_plugin_activate(const struct clap_plugin *plugin, double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Activating plugin (Sample Rate: %.2f, Min Frames: %u, Max Frames: %u)\n", sample_rate, min_frames_count, max_frames_count);
    
    try {
        // Update plugin state
        self->sampleRate = sample_rate;
        self->bufferSize = max_frames_count;
        
        // Configure audio processing through view model
        if (self->viewModel) {
            self->viewModel->setSampleRate(sample_rate);
            self->viewModel->setBufferSize(max_frames_count);
            self->viewModel->updateStatus("Activated");
        }
        
        return true;
    } catch (const std::exception& e) {
        printf("MyPlugin: Error during activation: %s\n", e.what());
        return false;
    }
}

static void my_plugin_deactivate(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Deactivating plugin\n");
    
    try {
        if (self->viewModel) {
            self->viewModel->updateStatus("Deactivated");
        }
        
        if (self->window) {
            self->window->hide();
        }
    } catch (const std::exception& e) {
        printf("MyPlugin: Error during deactivation: %s\n", e.what());
    }
}

static bool my_plugin_start_processing(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Starting processing\n");
    
    try {
        self->isProcessing = true;
        
        if (self->viewModel) {
            self->viewModel->updateStatus("Processing");
        }
        
        return true;
    } catch (const std::exception& e) {
        printf("MyPlugin: Error starting processing: %s\n", e.what());
        return false;
    }
}

static void my_plugin_stop_processing(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Stopping processing\n");
    
    try {
        self->isProcessing = false;
        
        if (self->viewModel) {
            self->viewModel->updateStatus("Stopped");
        }
    } catch (const std::exception& e) {
        printf("MyPlugin: Error stopping processing: %s\n", e.what());
    }
}

static void my_plugin_reset(const struct clap_plugin *plugin) {
    printf("MyPlugin: Resetting plugin\n");
    // Reset plugin state (e.g., clear buffers, reset parameters)
}

static clap_process_status my_plugin_process(const struct clap_plugin *plugin, const clap_process_t *process) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    try {
        // Process audio through MVVM architecture
        if (self->viewModel && process->audio_outputs_count > 0 && process->audio_inputs_count > 0) {
            clap_audio_buffer_t *out_buf = &process->audio_outputs[0];
            const clap_audio_buffer_t *in_buf = &process->audio_inputs[0];
            
            if (out_buf->channel_count >= 2 && in_buf->channel_count >= 2 && out_buf->data32 && in_buf->data32) {
                // Create audio buffer wrapper for MVVM processing
                audio::AudioBuffer buffer(process->frames_count, std::min(in_buf->channel_count, out_buf->channel_count));
                
                // Copy input to buffer
                for (uint32_t ch = 0; ch < buffer.channelCount; ++ch) {
                    for (uint32_t i = 0; i < process->frames_count; ++i) {
                        buffer.channels[ch][i] = in_buf->data32[ch][i];
                    }
                }
                
                // Process through view model
                self->viewModel->processAudio(buffer);
                
                // Copy processed buffer to output
                for (uint32_t ch = 0; ch < buffer.channelCount; ++ch) {
                    for (uint32_t i = 0; i < process->frames_count; ++i) {
                        out_buf->data32[ch][i] = buffer.channels[ch][i];
                    }
                }
            }
        }
        
        return CLAP_PROCESS_CONTINUE;
    } catch (const std::exception& e) {
        printf("MyPlugin: Error during processing: %s\n", e.what());
        return CLAP_PROCESS_ERROR;
    }
}

// GUI Extension callbacks
static bool gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    return strcmp(api, CLAP_WINDOW_API_WIN32) == 0 || 
           strcmp(api, CLAP_WINDOW_API_COCOA) == 0 || 
           strcmp(api, CLAP_WINDOW_API_X11) == 0;
}

static bool gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating) {
#ifdef _WIN32
    *api = CLAP_WINDOW_API_WIN32;
#elif __APPLE__
    *api = CLAP_WINDOW_API_COCOA;
#else
    *api = CLAP_WINDOW_API_X11;
#endif
    *is_floating = false;
    return true;
}

static bool gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    try {
        if (self->window) {
            printf("MyPlugin: GUI already created\n");
            return true;
        }
        
        printf("MyPlugin: Creating GUI with API: %s\n", api);
        // Window is already created in init, just need to show it
        return true;
    } catch (const std::exception& e) {
        printf("MyPlugin: Error creating GUI: %s\n", e.what());
        return false;
    }
}

static void gui_destroy(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    try {
        if (self->window) {
            self->window->hide();
        }
        printf("MyPlugin: GUI destroyed\n");
    } catch (const std::exception& e) {
        printf("MyPlugin: Error destroying GUI: %s\n", e.what());
    }
}

static bool gui_set_scale(const clap_plugin_t *plugin, double scale) {
    printf("MyPlugin: Set scale: %f\n", scale);
    return true; // Accept scaling but don't do anything for now
}

static bool gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    try {
        if (self->window) {
            self->window->setSize(width, height);
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        printf("MyPlugin: Error setting GUI size: %s\n", e.what());
        return false;
    }
}

static bool gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    *width = 400;
    *height = 300;
    return true;
}

static bool gui_can_resize(const clap_plugin_t *plugin) {
    return true;
}

static bool gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints) {
    hints->can_resize_horizontally = true;
    hints->can_resize_vertically = true;
    hints->preserve_aspect_ratio = false;
    hints->aspect_ratio_width = 4;
    hints->aspect_ratio_height = 3;
    return true;
}

static bool gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    if (*width < 200) *width = 200;
    if (*height < 150) *height = 150;
    if (*width > 800) *width = 800;
    if (*height > 600) *height = 600;
    return true;
}

static bool gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    try {
        if (self->window) {
            // Set parent window for embedding in DAW
            // This would need platform-specific implementation
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        printf("MyPlugin: Error setting GUI parent: %s\n", e.what());
        return false;
    }
}

static bool gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window) {
    return true;
}

static void gui_suggest_title(const clap_plugin_t *plugin, const char *title) {
    printf("MyPlugin: Suggested title: %s\n", title);
}

static bool gui_show(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    try {
        if (self->window) {
            self->window->show();
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        printf("MyPlugin: Error showing GUI: %s\n", e.what());
        return false;
    }
}

static bool gui_hide(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    try {
        if (self->window) {
            self->window->hide();
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        printf("MyPlugin: Error hiding GUI: %s\n", e.what());
        return false;
    }
}

static const struct clap_plugin_gui my_gui_extension = {
    gui_is_api_supported,
    gui_get_preferred_api,
    gui_create,
    gui_destroy,
    gui_set_scale,
    gui_get_size,
    gui_can_resize,
    gui_get_resize_hints,
    gui_adjust_size,
    gui_set_size,
    gui_set_parent,
    gui_set_transient,
    gui_suggest_title,
    gui_show,
    gui_hide,
};

static const void *my_plugin_get_extension(const struct clap_plugin *plugin, const char *id) {
    printf("MyPlugin: Host requesting extension: %s\n", id);
    
    if (strcmp(id, CLAP_EXT_GUI) == 0) {
        return &my_gui_extension;
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
