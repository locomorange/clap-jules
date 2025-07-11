#include "my_plugin.h"
#include "mvvm_impl.h"
#include "gui.h"
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc
#include <memory>
#include <vector>
#include <algorithm>
#include <clap/ext/gui.h> // CLAP GUI Extension header

#ifdef HAVE_BRISK
#include <brisk/gui/GUIApplication.hpp> // For GuiApplication
#endif

#ifdef HAVE_BOOST_DI
#include <boost/di.hpp>
namespace di = boost::di;
#endif

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

// --- CLAP GUI Extension Callbacks ---
static bool gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating);
static void gui_destroy(const clap_plugin_t *plugin);
static bool gui_set_scale(const clap_plugin_t *plugin, double scale);
static bool gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height);
static bool gui_can_resize(const clap_plugin_t *plugin);
static bool gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints);
static bool gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height);
static bool gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height);
static bool gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window);
static bool gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window);
static void gui_suggest_title(const clap_plugin_t *plugin, const char *title);
static bool gui_show(const clap_plugin_t *plugin);
static bool gui_hide(const clap_plugin_t *plugin);


// --- Plugin Descriptor ---
// Features array for the plugin descriptor
static const char *const plugin_features[] = {"audio_effect", "filter", nullptr};

static const clap_plugin_descriptor_t my_plugin_descriptor = {
    CLAP_VERSION,
    "com.example.myplugin", // id
    "My CLAP Filter Plugin with MVVM", // name
    "My Company",           // vendor
    "https://example.com",  // url
    "https://example.com/bugtracker", // manual_url
    "https://example.com/support",    // support_url
    "0.1.0",                // version
    "A CLAP audio plugin with KFR low-pass filter and MVVM architecture.", // description
    plugin_features, // features
};


// --- Plugin Implementation ---
static bool my_plugin_init(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Initializing plugin with MVVM architecture\n");
    
    // Initialize MVVM components using dependency injection
#ifdef HAVE_BOOST_DI
    printf("MyPlugin: Initializing with Boost.DI\n");
    auto injector = di::make_injector(
        di::bind<AudioModel>().to<FilterAudioModel>().in(di::singleton),
        di::bind<PluginViewModel>().to<FilterPluginViewModel>().in(di::singleton)
#ifdef HAVE_BRISK
        , di::bind<IGui>().to<BriskPluginGUI>().in(di::singleton)
#else
        , di::bind<IGui>().to<SimplePluginGUI>().in(di::singleton)
#endif
    );
    // Create main components through DI
    self->viewModel = injector.create<std::shared_ptr<PluginViewModel>>();
    self->gui = injector.create<std::shared_ptr<IGui>>();
#else
    // Fallback without dependency injection
    printf("MyPlugin: Initializing without Boost.DI (fallback)\n");
    auto model = std::make_shared<FilterAudioModel>();
    self->viewModel = std::make_shared<FilterPluginViewModel>(model);
#ifdef HAVE_BRISK
    self->gui = std::make_shared<BriskPluginGUI>();
#else
    self->gui = std::make_shared<SimplePluginGUI>();
#endif
#endif
    
    // Connect GUI to view model
    if (self->gui) {
        self->gui->setFrequencyChangeCallback([self](double frequency) {
            if (self->viewModel) {
                self->viewModel->setFrequency(frequency);
                self->currentFrequency = frequency; // Consider if this is still needed or if VM is sole source of truth
                printf("MyPlugin: Frequency changed to %.2f Hz via GUI callback\n", frequency);
            }
        });
        // If GUI needs direct access to ViewModel, it should be injected.
        // For now, callback is the primary interaction from GUI -> ViewModel.
    } else {
        printf("MyPlugin: Error - GUI component is null after initialization!\n");
    }

    if (!self->viewModel) {
        printf("MyPlugin: Error - ViewModel component is null after initialization!\n");
    }
    
    self->currentFrequency = 1000.0; // Default frequency
    return true;
}

static void my_plugin_destroy(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Destroying plugin\n");
    // Reset shared pointers to release resources
    self->gui.reset();
    self->viewModel.reset();
}

static bool my_plugin_activate(const struct clap_plugin *plugin, double sample_rate, uint32_t min_frames_count, uint32_t max_frames_count) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Activating plugin (Sample Rate: %.2f, Min Frames: %u, Max Frames: %u)\n", sample_rate, min_frames_count, max_frames_count);
    
    // Configure the view model with sample rate
    if (self->viewModel) {
        self->viewModel->setSampleRate(sample_rate);
        self->viewModel->setFrequency(self->currentFrequency);
    }
    
    // Update GUI
    if (self->gui) {
        self->gui->setFrequency(self->currentFrequency);
        self->gui->show(); // Show GUI when plugin is activated
    }
    
    return true;
}

static void my_plugin_deactivate(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Deactivating plugin\n");
    
    // Reset filter state
    if (self->viewModel) {
        self->viewModel->reset();
    }
    
    // Hide GUI
    if (self->gui) {
        self->gui->hide();
    }
}

static bool my_plugin_start_processing(const struct clap_plugin *plugin) {
    printf("MyPlugin: Starting processing\n");
    return true;
}

static void my_plugin_stop_processing(const struct clap_plugin *plugin) {
    printf("MyPlugin: Stopping processing\n");
}

static void my_plugin_reset(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Resetting plugin\n");
    
    // Reset plugin state through view model
    if (self->viewModel) {
        self->viewModel->reset();
    }
}

static clap_process_status my_plugin_process(const struct clap_plugin *plugin, const clap_process_t *process) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    // Process audio through MVVM architecture
    if (process->audio_outputs_count > 0 && process->audio_inputs_count > 0 && self->viewModel) {
        clap_audio_buffer_t *out_buf = &process->audio_outputs[0];
        const clap_audio_buffer_t *in_buf = &process->audio_inputs[0];

        if (out_buf->channel_count >= 1 && in_buf->channel_count >= 1 && out_buf->data32 && in_buf->data32) {
            // Create interleaved buffers for processing
            uint32_t channels = std::min(out_buf->channel_count, in_buf->channel_count);
            std::vector<float> interleavedInput(process->frames_count * channels);
            std::vector<float> interleavedOutput(process->frames_count * channels);
            
            // Interleave input
            for (uint32_t ch = 0; ch < channels; ++ch) {
                for (uint32_t i = 0; i < process->frames_count; ++i) {
                    interleavedInput[i * channels + ch] = in_buf->data32[ch][i];
                }
            }
            
            // Process through view model
            self->viewModel->processAudio(
                interleavedInput.data(), 
                interleavedOutput.data(), 
                process->frames_count, 
                channels
            );
            
            // De-interleave output
            for (uint32_t ch = 0; ch < channels; ++ch) {
                for (uint32_t i = 0; i < process->frames_count; ++i) {
                    out_buf->data32[ch][i] = interleavedOutput[i * channels + ch];
                }
            }
        }
    }
    
    return CLAP_PROCESS_CONTINUE;
}

static const void *my_plugin_get_extension(const struct clap_plugin *plugin, const char *id) {
    // Example: if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) return &my_audio_ports_extension;
    // Example: if (strcmp(id, CLAP_EXT_PARAMS) == 0) return &my_params_extension;

    if (strcmp(id, CLAP_EXT_GUI) == 0) {
        // BRISK_LOG_INFO("Host requested CLAP_EXT_GUI");
        printf("MyPlugin: Host requested CLAP_EXT_GUI\n");
        // Return the GUI extension struct
        static const clap_plugin_gui_t gui_extension = {
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
            gui_hide
        };
        return &gui_extension;
    }

    printf("MyPlugin: Host requesting unknown extension: %s\n", id);
    return NULL;
}

static void my_plugin_on_main_thread(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    // This is where GUI events could be processed if Brisk needs manual pumping,
    // but typically Brisk runs its own event loop on a thread or integrates with host's loop.
    // For CLAP, GUI operations are often driven by host calls to the GUI extension functions.
    // If Brisk needs periodic updates not tied to direct interactions, this could be a place.
    // brisk::Application::instance().processEvents(); // Example if Brisk needs this.
    // Called by the host to perform tasks that must run on the main thread.
    
    // Demo: Simulate some GUI interaction for testing
    static int call_count = 0;
    call_count++;
    
#ifndef HAVE_BRISK // This simulation is specific to SimplePluginGUI
    if (call_count == 10 && self->gui) {
        // Simulate user changing frequency after some time
        SimplePluginGUI* simpleGUI = dynamic_cast<SimplePluginGUI*>(self->gui.get());
        if (simpleGUI) simpleGUI->simulateFrequencyChange(500.0); // Change to 500 Hz
    } else if (call_count == 20 && self->gui) {
        SimplePluginGUI* simpleGUI = dynamic_cast<SimplePluginGUI*>(self->gui.get());
        if (simpleGUI) simpleGUI->simulateFrequencyChange(2000.0); // Change to 2000 Hz
    }
#endif
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
// Static instance for Brisk GuiApplication
#ifdef HAVE_BRISK
static std::unique_ptr<brisk::GuiApplication> g_brisk_gui_application;
#endif

// This is the main entry point that the host will look for.
CLAP_EXPORT const clap_plugin_entry_t clap_entry = {
    CLAP_VERSION,
    // init: Called once when the library is loaded.
    [](const char *plugin_path) -> bool {
        printf("MyPlugin: clap_entry.init called (path: %s)\n", plugin_path);
        #ifdef HAVE_BRISK
            printf("MyPlugin: Initializing Brisk GuiApplication...\n");
            // Catch exceptions during construction if any
            try {
                g_brisk_gui_application = std::make_unique<brisk::GuiApplication>();
                // Some toolkits require an explicit init call after construction,
                // but brisk-helloworld doesn't show it for GuiApplication.
                // If initialization is needed and can fail, check return status.
                printf("MyPlugin: Brisk GuiApplication initialized.\n");
            } catch (const std::exception& e) {
                fprintf(stderr, "MyPlugin: Failed to initialize Brisk GuiApplication: %s\n", e.what());
                return false; // Failed to initialize
            } catch (...) {
                fprintf(stderr, "MyPlugin: Failed to initialize Brisk GuiApplication due to unknown exception.\n");
                return false;
            }
        #endif
        return true;
    },
    // deinit: Called once when the library is unloaded.
    []() -> void {
        printf("MyPlugin: clap_entry.deinit called\n");
        #ifdef HAVE_BRISK
            printf("MyPlugin: Shutting down Brisk GuiApplication...\n");
            g_brisk_gui_application.reset(); // Destructor of GuiApplication should handle cleanup
            printf("MyPlugin: Brisk GuiApplication shut down.\n");
        #endif
        // Perform any other global library cleanup here if needed
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

// --- CLAP GUI Extension Callback Implementations ---

static bool gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (!self || !self->gui) return false;

    // Check if the GUI API is supported (e.g., CLAP_WINDOW_API_X11 on Linux)
    // For Brisk, it should handle various native backends.
    // We pass the clap_window_t (which includes parent handle and API) to BriskPluginGUI::create
    printf("MyPlugin: gui_create called. API: %s, is_floating: %d\n", api, is_floating);

    // The clap_window_t itself is passed in gui_set_parent or gui_set_transient by the host.
    // gui_create is more about checking if the plugin *can* create a GUI with the given API.
    // Brisk's EmbedWindow will need the actual parent handle from clap_window_t.
    // For now, we assume BriskPluginGUI::create will be called later via gui_set_parent.
    // However, some hosts might call create and expect immediate window setup if is_floating is true.
    // Let's defer actual Brisk window creation to gui_set_parent for embedded,
    // and potentially handle floating here if Brisk supports it directly.

    // For CLAP, the plugin should create its window content when gui_set_parent is called.
    // gui_create is more of a check. If Brisk needs global init, it could be here or in plugin_init.
    // For now, we rely on BriskPluginGUI::create to handle the embedding.
    // The bool return here often indicates if the plugin *can* provide a GUI of this type.
    return true;
}

static void gui_destroy(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: gui_destroy called\n");
    if (self && self->gui) {
        self->gui->destroy(); // Call the destroy method on our GUI object
    }
}

static bool gui_set_scale(const clap_plugin_t *plugin, double scale) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: gui_set_scale called with scale %.2f\n", scale);
    // TODO: If Brisk supports UI scaling, implement this.
    // if (self && self->gui) { return self->gui->set_scale(scale); }
    return false; // Not implemented yet
}

static bool gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (!self || !self->gui) return false;
    bool result = self->gui->get_size(width, height);
    // printf("MyPlugin: gui_get_size called. Reported: %u x %u\n", *width, *height);
    return result;
}

static bool gui_can_resize(const clap_plugin_t *plugin) {
    // TODO: Check if Brisk window is resizable. For now, assume false or a fixed size.
    printf("MyPlugin: gui_can_resize called\n");
    return false; // Or query self->gui if it has such a property
}

static bool gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints) {
    printf("MyPlugin: gui_get_resize_hints called\n");
    // TODO: Provide resize hints if resizable.
    return false;
}

static bool gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: gui_adjust_size called with preferred %u x %u\n", *width, *height);
    // TODO: Ask Brisk GUI to adjust to a preferred size, it might return a new valid size.
    // For now, let's say we stick to the requested size if possible, or our default.
    // if (self && self->gui) { return self->gui->adjust_size(width, height); }
    // If not implemented, returning false means the host should use get_size.
    return gui_get_size(plugin, width, height); // Fallback to current/default size
}

static bool gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (!self || !self->gui) return false;
    printf("MyPlugin: gui_set_size called with %u x %u\n", width, height);
    return self->gui->set_size(width, height);
}

static bool gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (!self || !self->gui) return false;
    printf("MyPlugin: gui_set_parent called. Parent HWND: %p, API: %s\n", window->parent, window->plugin_api);
    // This is where the actual Brisk window should be created and parented.
    return self->gui->create(window);
}

static bool gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: gui_set_transient called. Parent HWND: %p, API: %s\n", window->parent, window->plugin_api);
    // For floating windows. Brisk might handle this similarly to set_parent or have specific API.
    // If BriskPluginGUI::create can handle this (e.g. by creating a non-embedded window if window->parent is null)
    // if (self && self->gui) { return self->gui->create_transient(window); }
    return false; // Not implemented for now for transient windows
}

static void gui_suggest_title(const clap_plugin_t *plugin, const char *title) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: gui_suggest_title: %s\n", title);
    // TODO: If Brisk window is floating and has a title bar, set it.
    // if (self && self->gui) { self->gui->set_title(title); }
}

static bool gui_show(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (!self || !self->gui) return false;
    printf("MyPlugin: gui_show called\n");
    self->gui->show();
    return true;
}

static bool gui_hide(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (!self || !self->gui) return false;
    printf("MyPlugin: gui_hide called\n");
    self->gui->hide();
    return true;
}
