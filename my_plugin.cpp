#include "my_plugin.h"
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc

// OpenGL support for rendering
#ifdef HAVE_GLFW
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
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

#ifdef HAVE_GLFW
// GUI function declarations
static bool gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating);
static bool gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating);
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

// GLFW callbacks
static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
static void render_gui(my_plugin_t* plugin);
#endif

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
    
    // Initialize graphics libraries if available
#ifdef HAVE_GLFW
    // Initialize GUI state regardless of GLFW initialization success
    self->window = nullptr;
    self->gui_created = false;
    self->button_red = true;  // Start with red button
    self->window_width = 400;
    self->window_height = 300;
    
    if (glfwInit()) {
        printf("MyPlugin: GLFW initialized successfully\n");
    } else {
        printf("MyPlugin: Warning - GLFW initialization failed\n");
    }
#endif

#ifdef HAVE_SKIA
    printf("MyPlugin: Skia support is available\n");
    self->surface = nullptr;
#endif

    return true;
}

static void my_plugin_destroy(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Destroying plugin\n");
    
    // Cleanup graphics resources
#ifdef HAVE_GLFW
    if (self->window) {
        glfwDestroyWindow(self->window);
        self->window = nullptr;
    }
    glfwTerminate();
    printf("MyPlugin: GLFW terminated\n");
#endif

#ifdef HAVE_SKIA
    if (self->surface) {
        self->surface.reset();
    }
    printf("MyPlugin: Skia resources cleaned up\n");
#endif
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
#ifdef HAVE_GLFW
    if (strcmp(id, CLAP_EXT_GUI) == 0) {
        return &my_plugin_gui;
    }
#endif
    return NULL; // No other extensions supported in this basic example
}

static void my_plugin_on_main_thread(const struct clap_plugin *plugin) {
    // Called by the host to perform tasks that must run on the main thread.
#ifdef HAVE_GLFW
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self->window && !glfwWindowShouldClose(self->window)) {
        render_gui(self);
        glfwPollEvents();
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

const struct clap_plugin_factory my_plugin_factory = {
    my_factory_get_plugin_count,
    my_factory_get_plugin_descriptor,
    my_factory_create_plugin,
};

// Function to access the plugin factory
CLAP_EXPORT const struct clap_plugin_factory* get_plugin_factory() {
    return &my_plugin_factory;
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

#ifdef HAVE_GLFW
// --- GUI Extension Implementation ---

static bool gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    // We only support floating windows since we use GLFW
    return is_floating && (api == NULL || strlen(api) == 0);
}

static bool gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating) {
    *api = "";  // Use default API
    *is_floating = true;  // We create floating windows
    return true;
}

static bool gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    if (self->gui_created) {
        return false;  // GUI already created
    }
    
    if (!is_floating) {
        return false;  // We only support floating windows
    }
    
    // Create GLFW window
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);  // Start hidden
    self->window = glfwCreateWindow(self->window_width, self->window_height, "CLAP Plugin GUI", NULL, NULL);
    
    if (!self->window) {
        printf("MyPlugin: Failed to create GLFW window\n");
        return false;
    }
    
    // Set up OpenGL context
    glfwMakeContextCurrent(self->window);
    
    // Set up mouse callback
    glfwSetWindowUserPointer(self->window, self);
    glfwSetMouseButtonCallback(self->window, glfw_mouse_button_callback);
    
    // Enable VSync
    glfwSwapInterval(1);
    
    self->gui_created = true;
    printf("MyPlugin: GUI created successfully\n");
    return true;
}

static void gui_destroy(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    if (self->window) {
        glfwDestroyWindow(self->window);
        self->window = nullptr;
    }
    
    self->gui_created = false;
    printf("MyPlugin: GUI destroyed\n");
}

static bool gui_set_scale(const clap_plugin_t *plugin, double scale) {
    // We ignore scaling for this simple example
    return true;
}

static bool gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    *width = self->window_width;
    *height = self->window_height;
    return true;
}

static bool gui_can_resize(const clap_plugin_t *plugin) {
    return false;  // Fixed size for simplicity
}

static bool gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints) {
    return false;  // No resize hints
}

static bool gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    return false;  // Fixed size
}

static bool gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height) {
    return false;  // Fixed size
}

static bool gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    return false;  // We only support floating windows
}

static bool gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window) {
    // TODO: Implement window parent relationship
    return true;
}

static void gui_suggest_title(const clap_plugin_t *plugin, const char *title) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (self->window) {
        glfwSetWindowTitle(self->window, title);
    }
}

static bool gui_show(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    if (!self->window) {
        return false;
    }
    
    glfwShowWindow(self->window);
    printf("MyPlugin: GUI shown\n");
    return true;
}

static bool gui_hide(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    if (!self->window) {
        return false;
    }
    
    glfwHideWindow(self->window);
    printf("MyPlugin: GUI hidden\n");
    return true;
}

// GUI extension structure
const clap_plugin_gui_t my_plugin_gui = {
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

// GLFW mouse callback
static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        my_plugin_t* plugin = (my_plugin_t*)glfwGetWindowUserPointer(window);
        
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        // Check if click is within button area (center of window, 100x50 pixels)
        int window_width, window_height;
        glfwGetWindowSize(window, &window_width, &window_height);
        
        int button_x = window_width / 2 - 50;
        int button_y = window_height / 2 - 25;
        int button_width = 100;
        int button_height = 50;
        
        if (xpos >= button_x && xpos <= button_x + button_width &&
            ypos >= button_y && ypos <= button_y + button_height) {
            
            // Toggle button color
            plugin->button_red = !plugin->button_red;
            printf("MyPlugin: Button clicked! Color changed to %s\n", 
                   plugin->button_red ? "red" : "blue");
        }
    }
}

// Render the GUI
static void render_gui(my_plugin_t* plugin) {
    if (!plugin->window) {
        return;
    }
    
    glfwMakeContextCurrent(plugin->window);
    
    int window_width, window_height;
    glfwGetFramebufferSize(plugin->window, &window_width, &window_height);
    
    glViewport(0, 0, window_width, window_height);
    
    // Clear screen with dark gray
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Set up 2D rendering
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, window_width, window_height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Draw button
    int button_x = window_width / 2 - 50;
    int button_y = window_height / 2 - 25;
    int button_width = 100;
    int button_height = 50;
    
    // Set button color
    if (plugin->button_red) {
        glColor3f(0.8f, 0.2f, 0.2f);  // Red
    } else {
        glColor3f(0.2f, 0.2f, 0.8f);  // Blue
    }
    
    // Draw filled rectangle
    glBegin(GL_QUADS);
    glVertex2i(button_x, button_y);
    glVertex2i(button_x + button_width, button_y);
    glVertex2i(button_x + button_width, button_y + button_height);
    glVertex2i(button_x, button_y + button_height);
    glEnd();
    
    // Draw button border
    glColor3f(1.0f, 1.0f, 1.0f);  // White border
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(button_x, button_y);
    glVertex2i(button_x + button_width, button_y);
    glVertex2i(button_x + button_width, button_y + button_height);
    glVertex2i(button_x, button_y + button_height);
    glEnd();
    
    glfwSwapBuffers(plugin->window);
}

#endif
