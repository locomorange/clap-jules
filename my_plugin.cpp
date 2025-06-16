#include "my_plugin.h"
#include "graphics/skia_graphics.h"
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc
#include <cmath>    // For sin/cos
#include <memory>   // For std::make_unique, std::unique_ptr
#include <clap/ext/gui.h>

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

// --- Plugin-specific rendering functions ---
static void my_plugin_render_content(my_plugin_t *self);
static bool my_plugin_present_graphics(my_plugin_t *self);

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

// --- GUI Extension Implementation ---
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
    
    // Initialize GUI state
    self->gui_created = false;
    self->gui_visible = false;
    self->gui_width = 320;
    self->gui_height = 240;
    self->gui_api = nullptr;
    self->gui_is_floating = false;
    self->native_window = nullptr;
    self->needs_redraw = true;
#if defined(__linux__) && defined(HAVE_X11)
    self->x11_renderer = nullptr;
#endif
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    self->win32_renderer = nullptr;
#endif
    
    // Initialize graphics system and demonstrate basic usage
    printf("MyPlugin: Graphics backend - %s\n", clap_jules::graphics::getGraphicsBackendInfo().c_str());
    printf("MyPlugin: Skia available - %s\n", clap_jules::graphics::isSkiaAvailable() ? "Yes" : "No");
    
    // Create a test graphics context
    auto graphics = clap_jules::graphics::createGraphicsContext(320, 240);
    if (graphics) {
        // Demonstrate basic graphics operations
        graphics->clear(clap_jules::graphics::Color(50, 50, 50)); // Dark gray background
        graphics->drawRect(clap_jules::graphics::Rect(10, 10, 100, 50), 
                          clap_jules::graphics::Color(255, 100, 100)); // Red rectangle
        graphics->drawCircle(clap_jules::graphics::Point(200, 120), 30, 
                           clap_jules::graphics::Color(100, 255, 100)); // Green circle
        graphics->drawLine(clap_jules::graphics::Point(50, 200), 
                          clap_jules::graphics::Point(250, 200),
                          clap_jules::graphics::Color(100, 100, 255), 3.0f); // Blue line
        graphics->drawText("CLAP-Jules", clap_jules::graphics::Point(50, 150), 
                          clap_jules::graphics::Color(255, 255, 255), 16.0f); // White text
        
        printf("MyPlugin: Graphics context created and test drawing performed\n");
    }
    
    // Initialize your plugin state here
    return true;
}

static void my_plugin_destroy(const struct clap_plugin *plugin) {
    printf("MyPlugin: Destroying plugin\n");
    // Free any resources allocated in init
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
        printf("MyPlugin: Returning GUI extension\n");
        return &my_plugin_gui;
    }
    
    // Example: if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) return &my_audio_ports_extension;
    // Example: if (strcmp(id, CLAP_EXT_PARAMS) == 0) return &my_params_extension;
    return NULL; // Extension not supported
}

static void my_plugin_on_main_thread(const struct clap_plugin *plugin) {
    // Called by the host to perform tasks that must run on the main thread.
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    // If GUI is visible, update the rendering
    if (self->gui_created && self->gui_visible && self->graphics_context) {
        my_plugin_render_content(self);
        my_plugin_present_graphics(self);
    }
}

// --- Plugin-specific rendering functions ---

static void my_plugin_render_content(my_plugin_t *self) {
    if (!self->graphics_context) {
        return;
    }
    
    // Render the plugin's GUI content
    self->graphics_context->clear(clap_jules::graphics::Color(40, 40, 50)); // Dark blue-gray background
    
    // Draw UI elements
    self->graphics_context->drawRect(clap_jules::graphics::Rect(10, 10, 120, 60), 
                                   clap_jules::graphics::Color(80, 120, 200)); // Blue rectangle
    self->graphics_context->drawCircle(clap_jules::graphics::Point(self->gui_width/2, self->gui_height/2), 40, 
                                     clap_jules::graphics::Color(120, 200, 120)); // Green circle
    self->graphics_context->drawLine(clap_jules::graphics::Point(20, self->gui_height - 50), 
                                   clap_jules::graphics::Point(self->gui_width - 20, self->gui_height - 50),
                                   clap_jules::graphics::Color(200, 100, 255), 3.0f); // Purple line
    self->graphics_context->drawText("CLAP-Jules GUI", clap_jules::graphics::Point(20, self->gui_height - 20), 
                                   clap_jules::graphics::Color(255, 255, 255), 20.0f); // White text
    
    // Add some animated content to demonstrate it's working
    static int frame_counter = 0;
    frame_counter++;
    
    // Draw a small animated square
    float x = 50 + 30 * sin(frame_counter * 0.1f);
    float y = 80 + 20 * cos(frame_counter * 0.1f);
    self->graphics_context->drawRect(clap_jules::graphics::Rect(x, y, 20, 20), 
                                   clap_jules::graphics::Color(255, 200, 100)); // Animated yellow square
    
    // Finalize rendering
    self->graphics_context->present();
    
    printf("MyPlugin: Rendered frame %d at size %ux%u\n", frame_counter, self->gui_width, self->gui_height);
}

static bool my_plugin_present_graphics(my_plugin_t *self) {
    if (!self->graphics_context || !self->gui_created) {
        return false;
    }
    
    // Get the rendered pixel data
    const void* pixel_data = self->graphics_context->getPixelData();
    if (!pixel_data) {
        return false;
    }
    
    int width = self->graphics_context->getWidth();
    int height = self->graphics_context->getHeight();
    
    printf("MyPlugin: Presenting %ux%u graphics buffer to window\n", width, height);
    
#if defined(__linux__) && defined(HAVE_X11)
    // Use X11 renderer if available
    if (self->x11_renderer && self->x11_renderer->isInitialized()) {
        const uint32_t* pixels = static_cast<const uint32_t*>(pixel_data);
        return self->x11_renderer->presentPixelBuffer(pixels, width, height);
    }
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    // Use Win32 renderer if available
    if (self->win32_renderer && self->win32_renderer->isInitialized()) {
        const uint32_t* pixels = static_cast<const uint32_t*>(pixel_data);
        return self->win32_renderer->presentPixelBuffer(pixels, width, height);
    }
#endif
    
    // Fallback: just log that we would present the graphics
    printf("MyPlugin: Would present graphics buffer (no platform renderer available)\n");
    return true;
}

// --- GUI Extension Implementation ---

static bool my_plugin_gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    printf("MyPlugin: GUI - Checking API support: %s (floating: %s)\n", api, is_floating ? "yes" : "no");
    
    // Support the most common windowing APIs
    if (strcmp(api, CLAP_WINDOW_API_X11) == 0) return true;      // Linux X11
    if (strcmp(api, CLAP_WINDOW_API_WIN32) == 0) return true;    // Windows
    if (strcmp(api, CLAP_WINDOW_API_COCOA) == 0) return true;    // macOS
    if (strcmp(api, CLAP_WINDOW_API_WAYLAND) == 0 && is_floating) return true; // Wayland (floating only)
    
    return false;
}

static bool my_plugin_gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating) {
    printf("MyPlugin: GUI - Getting preferred API\n");
    
    // Prefer embedded windows on most platforms
    *is_floating = false;
    
    #ifdef __linux__
        *api = CLAP_WINDOW_API_X11;
    #elif defined(_WIN32)
        *api = CLAP_WINDOW_API_WIN32;
    #elif defined(__APPLE__)
        *api = CLAP_WINDOW_API_COCOA;
    #else
        *api = CLAP_WINDOW_API_X11; // Default fallback
    #endif
    
    printf("MyPlugin: GUI - Preferred API: %s\n", *api);
    return true;
}

static bool my_plugin_gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI - Creating window (API: %s, floating: %s)\n", api ? api : "none", is_floating ? "yes" : "no");
    
    if (self->gui_created) {
        printf("MyPlugin: GUI - Already created\n");
        return false;
    }
    
    // Store GUI settings
    self->gui_api = api;
    self->gui_is_floating = is_floating;
    
    // Create graphics context for the GUI
    self->graphics_context = clap_jules::graphics::createGraphicsContext(self->gui_width, self->gui_height);
    if (!self->graphics_context) {
        printf("MyPlugin: GUI - Failed to create graphics context\n");
        return false;
    }
    
    // Render initial content
    my_plugin_render_content(self);
    
    self->gui_created = true;
    self->needs_redraw = true;
    printf("MyPlugin: GUI - Window created successfully\n");
    return true;
}

static void my_plugin_gui_destroy(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI - Destroying window\n");
    
    if (!self->gui_created) {
        return;
    }
    
    // Clean up graphics context
    self->graphics_context.reset();
    
#if defined(__linux__) && defined(HAVE_X11)
    // Clean up X11 renderer
    self->x11_renderer.reset();
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    // Clean up Win32 renderer
    self->win32_renderer.reset();
#endif
    
    self->gui_created = false;
    self->gui_visible = false;
    self->gui_api = nullptr;
    
    printf("MyPlugin: GUI - Window destroyed\n");
}

static bool my_plugin_gui_set_scale(const clap_plugin_t *plugin, double scale) {
    printf("MyPlugin: GUI - Setting scale: %.2f\n", scale);
    // For now, we ignore scaling but report success
    return true;
}

static bool my_plugin_gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    *width = self->gui_width;
    *height = self->gui_height;
    printf("MyPlugin: GUI - Reporting size: %ux%u\n", *width, *height);
    return true;
}

static bool my_plugin_gui_can_resize(const clap_plugin_t *plugin) {
    printf("MyPlugin: GUI - Can resize: yes\n");
    return true; // Allow resizing
}

static bool my_plugin_gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints) {
    printf("MyPlugin: GUI - Getting resize hints\n");
    hints->can_resize_horizontally = true;
    hints->can_resize_vertically = true;
    hints->preserve_aspect_ratio = false;
    hints->aspect_ratio_width = 4;
    hints->aspect_ratio_height = 3;
    return true;
}

static bool my_plugin_gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    printf("MyPlugin: GUI - Adjusting size from %ux%u", *width, *height);
    
    // Constrain minimum size
    if (*width < 200) *width = 200;
    if (*height < 150) *height = 150;
    
    // Constrain maximum size
    if (*width > 1200) *width = 1200;
    if (*height > 800) *height = 800;
    
    printf(" to %ux%u\n", *width, *height);
    return true;
}

static bool my_plugin_gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI - Setting size: %ux%u\n", width, height);
    
    self->gui_width = width;
    self->gui_height = height;
    
    // Resize X11 renderer if available
#if defined(__linux__) && defined(HAVE_X11)
    if (self->x11_renderer && self->x11_renderer->isInitialized()) {
        self->x11_renderer->resize(width, height);
    }
#endif
    
    // Recreate graphics context with new size if GUI is created
    if (self->gui_created) {
        self->graphics_context = clap_jules::graphics::createGraphicsContext(width, height);
        if (self->graphics_context) {
            // Re-render content at new size
            my_plugin_render_content(self);
            my_plugin_present_graphics(self);
            self->needs_redraw = true;
        }
    }
    
    return true;
}

static bool my_plugin_gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI - Setting parent window (API: %s)\n", window ? window->api : "null");
    
    if (!window) {
        printf("MyPlugin: GUI - No parent window provided\n");
        return false;
    }
    
    // Store the native window handle for later use
    if (strcmp(window->api, CLAP_WINDOW_API_X11) == 0) {
        self->native_window = (void*)window->x11;
        printf("MyPlugin: GUI - X11 window handle: %lu\n", window->x11);
        
#if defined(__linux__) && defined(HAVE_X11)
        // Initialize X11 renderer
        self->x11_renderer = std::make_unique<clap_jules::graphics::X11Renderer>();
        if (!self->x11_renderer->initialize(window->x11, self->gui_width, self->gui_height)) {
            printf("MyPlugin: GUI - Failed to initialize X11 renderer\n");
            self->x11_renderer.reset();
        } else {
            printf("MyPlugin: GUI - X11 renderer initialized successfully\n");
        }
#endif
        
    } else if (strcmp(window->api, CLAP_WINDOW_API_WIN32) == 0) {
        self->native_window = window->win32;
        printf("MyPlugin: GUI - Win32 window handle set\n");
        
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
        // Initialize Win32 renderer
        self->win32_renderer = std::make_unique<clap_jules::graphics::Win32Renderer>();
        if (!self->win32_renderer->initialize((HWND)window->win32, self->gui_width, self->gui_height)) {
            printf("MyPlugin: GUI - Failed to initialize Win32 renderer\n");
            self->win32_renderer.reset();
        } else {
            printf("MyPlugin: GUI - Win32 renderer initialized successfully\n");
        }
#endif
    } else if (strcmp(window->api, CLAP_WINDOW_API_COCOA) == 0) {
        self->native_window = window->cocoa;
        printf("MyPlugin: GUI - Cocoa window handle set\n");
    }
    
    // Trigger initial render
    if (self->gui_created) {
        my_plugin_render_content(self);
        my_plugin_present_graphics(self);
    }
    
    printf("MyPlugin: GUI - Parent window set and initial render performed\n");
    return true;
}

static bool my_plugin_gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window) {
    printf("MyPlugin: GUI - Setting transient parent for floating window\n");
    // For floating windows, make the plugin window stay above the parent
    return true;
}

static void my_plugin_gui_suggest_title(const clap_plugin_t *plugin, const char *title) {
    printf("MyPlugin: GUI - Suggested title: %s\n", title ? title : "null");
    // For floating windows, set the window title
}

static bool my_plugin_gui_show(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI - Showing window\n");
    
    if (!self->gui_created) {
        printf("MyPlugin: GUI - Cannot show, window not created\n");
        return false;
    }
    
    self->gui_visible = true;
    
    // Render and present graphics when showing
    my_plugin_render_content(self);
    my_plugin_present_graphics(self);
    
    printf("MyPlugin: GUI - Window is now visible with rendered content\n");
    return true;
}

static bool my_plugin_gui_hide(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI - Hiding window\n");
    
    if (!self->gui_created) {
        return false;
    }
    
    self->gui_visible = false;
    printf("MyPlugin: GUI - Window is now hidden\n");
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
