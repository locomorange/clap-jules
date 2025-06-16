#include "my_plugin.h"
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc

#ifdef CLAP_JULES_WITH_SKIA
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#endif

// Platform-specific includes for GUI
#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_MAC
// macOS includes would go here
#endif
#elif defined(__linux__)
// X11 includes are conditional - we'll handle this gracefully
// #include <X11/Xlib.h>
// #include <X11/Xutil.h>
#endif

// --- GUI Data Structure ---
typedef struct gui_data {
    bool is_created;
    bool is_visible;
    bool is_floating;
    const char* api;
    uint32_t width;
    uint32_t height;
    
    // Platform-specific window handles
#ifdef _WIN32
    HWND hwnd;
    HWND parent_hwnd;
#elif defined(__APPLE__)
    void* nsview;
    void* parent_nsview;
#elif defined(__linux__)
    void* display;
    unsigned long window;
    unsigned long parent_window;
#endif

#ifdef CLAP_JULES_WITH_SKIA
    // Skia-specific drawing data
    sk_sp<SkSurface> surface;
#endif
} gui_data_t;

// --- GUI Extension Function Declarations ---
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

// GUI Extension Structure
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
    
    // Initialize GUI data
    self->gui_data = (gui_data_t *)calloc(1, sizeof(gui_data_t));
    if (!self->gui_data) {
        printf("MyPlugin: Failed to allocate GUI data\n");
        return false;
    }
    
    // Set default GUI parameters
    self->gui_data->width = 400;
    self->gui_data->height = 300;
    self->gui_data->is_created = false;
    self->gui_data->is_visible = false;
    
#ifdef CLAP_JULES_WITH_SKIA
    printf("MyPlugin: Skia integration enabled\n");
    
    // Simple Skia initialization test
    SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
    sk_sp<SkSurface> surface = SkSurface::MakeRaster(info);
    if (surface) {
        SkCanvas* canvas = surface->getCanvas();
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        canvas->drawRect(SkRect::MakeXYWH(10, 10, 80, 80), paint);
        printf("MyPlugin: Skia surface creation and drawing test successful\n");
    } else {
        printf("MyPlugin: Skia surface creation failed\n");
    }
#else
    printf("MyPlugin: Skia integration not enabled\n");
#endif
    
    printf("MyPlugin: Plugin initialized with GUI support\n");
    return true;
}

static void my_plugin_destroy(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Destroying plugin\n");
    
    // Clean up GUI data
    if (self->gui_data) {
        if (self->gui_data->is_created) {
            my_plugin_gui_destroy(plugin);
        }
        free(self->gui_data);
        self->gui_data = NULL;
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

static const void *my_plugin_get_extension(const struct clap_plugin *plugin, const char *id) {
    printf("MyPlugin: Host requesting extension: %s\n", id);
    
    if (strcmp(id, CLAP_EXT_GUI) == 0) {
        return &my_plugin_gui_extension;
    }
    
    return NULL; // Extension not supported
}

static void my_plugin_on_main_thread(const struct clap_plugin *plugin) {
    // Called by the host to perform tasks that must run on the main thread.
    // printf("MyPlugin: on_main_thread called\n");
}

// --- GUI Extension Implementation ---

static bool my_plugin_gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    printf("MyPlugin GUI: Checking API support for %s (floating: %s)\n", api, is_floating ? "yes" : "no");
    
    if (!api) {
        return is_floating; // For floating windows, API can be null
    }
    
#ifdef _WIN32
    if (strcmp(api, CLAP_WINDOW_API_WIN32) == 0) {
        return true;
    }
#elif defined(__APPLE__)
    if (strcmp(api, CLAP_WINDOW_API_COCOA) == 0) {
        return true;
    }
#elif defined(__linux__)
    if (strcmp(api, CLAP_WINDOW_API_X11) == 0) {
        return true;
    }
#endif
    
    return false;
}

static bool my_plugin_gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating) {
    printf("MyPlugin GUI: Getting preferred API\n");
    
    *is_floating = false; // Prefer embedded windows
    
#ifdef _WIN32
    *api = CLAP_WINDOW_API_WIN32;
#elif defined(__APPLE__)
    *api = CLAP_WINDOW_API_COCOA;
#elif defined(__linux__)
    *api = CLAP_WINDOW_API_X11;
#else
    *is_floating = true;
    *api = nullptr;
#endif
    
    return true;
}

static bool my_plugin_gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    gui_data_t *gui = self->gui_data;
    
    printf("MyPlugin GUI: Creating GUI (API: %s, floating: %s)\n", 
           api ? api : "null", is_floating ? "yes" : "no");
    
    if (gui->is_created) {
        printf("MyPlugin GUI: GUI already created\n");
        return false;
    }
    
    gui->api = api;
    gui->is_floating = is_floating;
    gui->is_created = true;
    gui->is_visible = false;
    
#ifdef CLAP_JULES_WITH_SKIA
    // Create Skia surface for drawing
    SkImageInfo info = SkImageInfo::MakeN32Premul(gui->width, gui->height);
    gui->surface = SkSurface::MakeRaster(info);
    if (gui->surface) {
        printf("MyPlugin GUI: Skia surface created successfully\n");
    }
#endif
    
    printf("MyPlugin GUI: GUI created successfully\n");
    return true;
}

static void my_plugin_gui_destroy(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    gui_data_t *gui = self->gui_data;
    
    printf("MyPlugin GUI: Destroying GUI\n");
    
    if (!gui->is_created) {
        return;
    }
    
    if (gui->is_visible) {
        my_plugin_gui_hide(plugin);
    }
    
#ifdef CLAP_JULES_WITH_SKIA
    gui->surface.reset();
#endif
    
    // Platform-specific cleanup would go here
    
    gui->is_created = false;
    printf("MyPlugin GUI: GUI destroyed\n");
}

static bool my_plugin_gui_set_scale(const clap_plugin_t *plugin, double scale) {
    printf("MyPlugin GUI: Set scale to %.2f\n", scale);
    // For this basic implementation, we ignore scaling
    return true;
}

static bool my_plugin_gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    gui_data_t *gui = self->gui_data;
    
    *width = gui->width;
    *height = gui->height;
    
    printf("MyPlugin GUI: Returning size %ux%u\n", *width, *height);
    return true;
}

static bool my_plugin_gui_can_resize(const clap_plugin_t *plugin) {
    printf("MyPlugin GUI: Checking if resizable\n");
    return true; // Allow resizing for this basic implementation
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
    printf("MyPlugin GUI: Adjusting size %ux%u\n", *width, *height);
    
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
    gui_data_t *gui = self->gui_data;
    
    printf("MyPlugin GUI: Setting size to %ux%u\n", width, height);
    
    gui->width = width;
    gui->height = height;
    
#ifdef CLAP_JULES_WITH_SKIA
    // Recreate Skia surface with new size
    if (gui->surface) {
        SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
        gui->surface = SkSurface::MakeRaster(info);
    }
#endif
    
    return true;
}

static bool my_plugin_gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    printf("MyPlugin GUI: Setting parent window (API: %s)\n", window->api);
    
    // In a real implementation, this would embed the plugin window into the host window
    // For now, we just store the parent information
    
    return true;
}

static bool my_plugin_gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window) {
    printf("MyPlugin GUI: Setting transient parent\n");
    
    // For floating windows, set the parent relationship
    return true;
}

static void my_plugin_gui_suggest_title(const clap_plugin_t *plugin, const char *title) {
    printf("MyPlugin GUI: Suggested title: %s\n", title);
}

static bool my_plugin_gui_show(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    gui_data_t *gui = self->gui_data;
    
    printf("MyPlugin GUI: Showing GUI\n");
    
    if (!gui->is_created) {
        printf("MyPlugin GUI: Cannot show - GUI not created\n");
        return false;
    }
    
    if (gui->is_visible) {
        printf("MyPlugin GUI: GUI already visible\n");
        return true;
    }
    
#ifdef CLAP_JULES_WITH_SKIA
    // Draw something with Skia if available
    if (gui->surface) {
        SkCanvas* canvas = gui->surface->getCanvas();
        
        // Clear the surface
        canvas->clear(SK_ColorWHITE);
        
        // Draw a simple interface
        SkPaint paint;
        
        // Draw background
        paint.setColor(SK_ColorLTGRAY);
        canvas->drawRect(SkRect::MakeWH(gui->width, gui->height), paint);
        
        // Draw title bar
        paint.setColor(SK_ColorBLUE);
        canvas->drawRect(SkRect::MakeWH(gui->width, 30), paint);
        
        // Draw some controls
        paint.setColor(SK_ColorRED);
        canvas->drawRect(SkRect::MakeXYWH(50, 60, 100, 50), paint);
        
        paint.setColor(SK_ColorGREEN);
        canvas->drawRect(SkRect::MakeXYWH(200, 60, 100, 50), paint);
        
        printf("MyPlugin GUI: Drew interface with Skia\n");
    }
#endif
    
    gui->is_visible = true;
    printf("MyPlugin GUI: GUI is now visible\n");
    return true;
}

static bool my_plugin_gui_hide(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    gui_data_t *gui = self->gui_data;
    
    printf("MyPlugin GUI: Hiding GUI\n");
    
    if (!gui->is_visible) {
        printf("MyPlugin GUI: GUI already hidden\n");
        return true;
    }
    
    gui->is_visible = false;
    printf("MyPlugin GUI: GUI is now hidden\n");
    return true;
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
