#include "my_plugin.h"
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc
#include <clap/ext/gui.h>

// GLFW includes
#ifdef __linux__
#define GLFW_EXPOSE_NATIVE_X11
#include <X11/Xlib.h>
#endif
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// OpenGL for basic rendering
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
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

// --- Forward declarations of GUI functions ---
static void my_plugin_gui_render(const clap_plugin_t *plugin);
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
    
    // Initialize GUI-related fields
    self->window = NULL;
    self->gui_created = false;
    self->gui_visible = false;
    self->gui_width = 400;
    self->gui_height = 300;
    self->gui_api = NULL;
    self->is_floating = false;
    
    // Initialize GLFW if not already done
    if (!glfwInit()) {
        printf("MyPlugin: Failed to initialize GLFW\n");
        return false;
    }
    
    return true;
}

static void my_plugin_destroy(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Destroying plugin\n");
    
    // Clean up GUI resources
    if (self->gui_created) {
        my_plugin_gui_destroy(plugin);
    }
    
    // Terminate GLFW
    glfwTerminate();
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

static const void *my_plugin_get_extension(const struct clap_plugin *plugin, const char *id) {
    // Return GUI extension if requested
    if (strcmp(id, CLAP_EXT_GUI) == 0) {
        return &my_plugin_gui_extension;
    }
    
    // Example: if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) return &my_audio_ports_extension;
    // Example: if (strcmp(id, CLAP_EXT_PARAMS) == 0) return &my_params_extension;
    printf("MyPlugin: Host requesting extension: %s\n", id);
    return NULL; // Extension not supported
}

static void my_plugin_on_main_thread(const struct clap_plugin *plugin) {
    // Called by the host to perform tasks that must run on the main thread.
    // printf("MyPlugin: on_main_thread called\n");
}

// --- GUI Extension Implementation ---

static void my_plugin_gui_render(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    if (!self->gui_created || !self->window) {
        return;
    }
    
    glfwMakeContextCurrent(self->window);
    
    // Get framebuffer size for proper rendering
    int width, height;
    glfwGetFramebufferSize(self->window, &width, &height);
    glViewport(0, 0, width, height);
    
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Basic OpenGL rendering - draw a simple gradient background
    glBegin(GL_TRIANGLES);
    
    // Draw two triangles to form a rectangle with gradient
    // Triangle 1
    glColor3f(0.2f, 0.3f, 0.8f); // Blue
    glVertex2f(-1.0f, -1.0f);     // Bottom left
    
    glColor3f(0.8f, 0.2f, 0.3f); // Red
    glVertex2f(1.0f, -1.0f);      // Bottom right
    
    glColor3f(0.3f, 0.8f, 0.2f); // Green
    glVertex2f(-1.0f, 1.0f);      // Top left
    
    // Triangle 2
    glColor3f(0.8f, 0.2f, 0.3f); // Red
    glVertex2f(1.0f, -1.0f);      // Bottom right
    
    glColor3f(0.3f, 0.8f, 0.2f); // Green
    glVertex2f(-1.0f, 1.0f);      // Top left
    
    glColor3f(0.8f, 0.8f, 0.2f); // Yellow
    glVertex2f(1.0f, 1.0f);       // Top right
    
    glEnd();
    
    // Draw a simple white rectangle in the center
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(-0.5f, -0.3f);
    glVertex2f(0.5f, -0.3f);
    glVertex2f(0.5f, 0.3f);
    glVertex2f(-0.5f, 0.3f);
    glEnd();
    
    // Add some text indication (using basic OpenGL - no text rendering for now)
    // Draw some lines to indicate this is a plugin GUI
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    
    // Draw a cross in the center
    glVertex2f(-0.2f, 0.0f);
    glVertex2f(0.2f, 0.0f);
    glVertex2f(0.0f, -0.2f);
    glVertex2f(0.0f, 0.2f);
    
    glEnd();
    
    // Swap buffers to display the rendered frame
    glfwSwapBuffers(self->window);
}

static bool my_plugin_gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    printf("MyPlugin: GUI API support check - API: %s, Floating: %s\n", api, is_floating ? "true" : "false");
    
    // Support X11 for Linux (embedded and floating)
    if (strcmp(api, CLAP_WINDOW_API_X11) == 0) {
        return true;
    }
    
    // Support Win32 for Windows (embedded and floating)
    if (strcmp(api, CLAP_WINDOW_API_WIN32) == 0) {
        return true;
    }
    
    return false;
}

static bool my_plugin_gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating) {
    printf("MyPlugin: GUI preferred API request\n");
    
    // Prefer embedded windows
    *is_floating = false;
    
    // Prefer X11 on Linux, Win32 on Windows
#ifdef __linux__
    *api = CLAP_WINDOW_API_X11;
#elif defined(_WIN32)
    *api = CLAP_WINDOW_API_WIN32;
#else
    *api = CLAP_WINDOW_API_X11; // Default to X11
#endif
    
    return true;
}

static bool my_plugin_gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI create - API: %s, Floating: %s\n", api ? api : "NULL", is_floating ? "true" : "false");
    
    if (self->gui_created) {
        printf("MyPlugin: GUI already created\n");
        return false;
    }
    
    // Store GUI configuration
    self->gui_api = api;
    self->is_floating = is_floating;
    
    // Configure GLFW window hints
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Start hidden
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    // Create GLFW window
    self->window = glfwCreateWindow(
        self->gui_width, 
        self->gui_height, 
        "My CLAP Plugin", 
        NULL, 
        NULL
    );
    
    if (!self->window) {
        printf("MyPlugin: Failed to create GLFW window\n");
        return false;
    }
    
    // Make context current for OpenGL operations
    glfwMakeContextCurrent(self->window);
    
    // Enable V-Sync
    glfwSwapInterval(1);
    
    // Set up basic OpenGL state
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f); // Dark blue background
    
    self->gui_created = true;
    printf("MyPlugin: GUI created successfully\n");
    return true;
}

static void my_plugin_gui_destroy(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI destroy\n");
    
    if (!self->gui_created) {
        return;
    }
    
    if (self->window) {
        glfwDestroyWindow(self->window);
        self->window = NULL;
    }
    
    self->gui_created = false;
    self->gui_visible = false;
}

static bool my_plugin_gui_set_scale(const clap_plugin_t *plugin, double scale) {
    printf("MyPlugin: GUI set scale: %f\n", scale);
    // For this simple implementation, we'll ignore scaling
    return true;
}

static bool my_plugin_gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    if (!self->gui_created) {
        *width = self->gui_width;
        *height = self->gui_height;
    } else {
        int w, h;
        glfwGetWindowSize(self->window, &w, &h);
        *width = (uint32_t)w;
        *height = (uint32_t)h;
    }
    
    printf("MyPlugin: GUI get size: %ux%u\n", *width, *height);
    return true;
}

static bool my_plugin_gui_can_resize(const clap_plugin_t *plugin) {
    printf("MyPlugin: GUI can resize: true\n");
    return true;
}

static bool my_plugin_gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints) {
    printf("MyPlugin: GUI get resize hints\n");
    hints->can_resize_horizontally = true;
    hints->can_resize_vertically = true;
    hints->preserve_aspect_ratio = false;
    return true;
}

static bool my_plugin_gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    printf("MyPlugin: GUI adjust size: %ux%u\n", *width, *height);
    
    // Ensure minimum size
    if (*width < 200) *width = 200;
    if (*height < 150) *height = 150;
    
    return true;
}

static bool my_plugin_gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI set size: %ux%u\n", width, height);
    
    self->gui_width = width;
    self->gui_height = height;
    
    if (self->window) {
        glfwSetWindowSize(self->window, width, height);
    }
    
    return true;
}

static bool my_plugin_gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI set parent - API: %s\n", window->api);
    
    if (!self->gui_created || !self->window) {
        printf("MyPlugin: GUI not created, cannot set parent\n");
        return false;
    }
    
    // Store parent window information
    self->parent_window = *window;
    
    // Handle X11 embedding
    if (strcmp(window->api, CLAP_WINDOW_API_X11) == 0) {
#ifdef __linux__
        Display* display = glfwGetX11Display();
        Window child = glfwGetX11Window(self->window);
        Window parent = (Window)window->x11;
        
        if (display && child && parent) {
            // Reparent the GLFW window
            XReparentWindow(display, child, parent, 0, 0);
            XMapWindow(display, child);
            XFlush(display);
            printf("MyPlugin: X11 window reparented successfully\n");
            return true;
        } else {
            printf("MyPlugin: Failed to get X11 handles\n");
            return false;
        }
#else
        printf("MyPlugin: X11 not supported on this platform\n");
        return false;
#endif
    }
    
    // Handle Win32 embedding
    if (strcmp(window->api, CLAP_WINDOW_API_WIN32) == 0) {
#ifdef _WIN32
        HWND child = glfwGetWin32Window(self->window);
        HWND parent = (HWND)window->win32;
        
        if (child && parent) {
            SetParent(child, parent);
            printf("MyPlugin: Win32 window reparented successfully\n");
            return true;
        } else {
            printf("MyPlugin: Failed to get Win32 handles\n");
            return false;
        }
#else
        printf("MyPlugin: Win32 not supported on this platform\n");
        return false;
#endif
    }
    
    printf("MyPlugin: Unsupported window API: %s\n", window->api);
    return false;
}

static bool my_plugin_gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window) {
    printf("MyPlugin: GUI set transient (floating window)\n");
    // For floating windows, we could set the window to stay above the parent
    // This is a simplified implementation
    return true;
}

static void my_plugin_gui_suggest_title(const clap_plugin_t *plugin, const char *title) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI suggest title: %s\n", title);
    
    if (self->window) {
        glfwSetWindowTitle(self->window, title);
    }
}

static bool my_plugin_gui_show(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI show\n");
    
    if (!self->gui_created || !self->window) {
        printf("MyPlugin: GUI not created, cannot show\n");
        return false;
    }
    
    glfwShowWindow(self->window);
    self->gui_visible = true;
    
    // Render initial frame
    my_plugin_gui_render(plugin);
    
    return true;
}

static bool my_plugin_gui_hide(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: GUI hide\n");
    
    if (!self->gui_created || !self->window) {
        return false;
    }
    
    glfwHideWindow(self->window);
    self->gui_visible = false;
    
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
