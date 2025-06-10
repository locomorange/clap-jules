#include "my_plugin.h"
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc
#include <GLFW/glfw3.h> // For ImGui window
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

// --- Plugin Descriptor ---
static const clap_plugin_descriptor_t my_plugin_descriptor = {
    CLAP_VERSION,
    "com.example.myplugin", // id
    "My First CLAP Plugin", // name
    "My Company",           // vendor
    "https://example.com",  // url
    "https://example.com/bugtracker", // manual_url
    "https://example.com/support",    // support_url
    "0.0.1",                // version
    "A simple example CLAP audio plugin with ImGui GUI.", // description
    (const char *const[]){"audio_effect", "gui", nullptr}, // features. Added "gui"
    // CLAP_PLUGIN_FEATURE_AUDIO_EFFECT, // Example if using clap_plugin_features.h
};

// --- CLAP GUI Extension Implementation ---

static bool gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    printf("MyPlugin GUI: gui_is_api_supported called (api: %s, is_floating: %s)\n", api, is_floating ? "true" : "false");
    if (is_floating) { // For floating windows, we handle it ourselves with GLFW/ImGui
        // Potentially support any API if we create our own window context.
        // For now, let's be explicit.
        if (strcmp(api, CLAP_WINDOW_API_X11) == 0 ||
            strcmp(api, CLAP_WINDOW_API_WIN32) == 0 ||
            strcmp(api, CLAP_WINDOW_API_COCOA) == 0) {
            return true;
        }
    } else { // For embedded windows
        if (strcmp(api, CLAP_WINDOW_API_X11) == 0 ||
            strcmp(api, CLAP_WINDOW_API_WIN32) == 0 ||
            strcmp(api, CLAP_WINDOW_API_COCOA) == 0) {
            return true; // We can try to embed into these
        }
    }
    return false;
}

static bool gui_get_preferred_api(const clap_plugin_t* plugin, const char** api, bool* is_floating) {
    printf("MyPlugin GUI: gui_get_preferred_api called\n");
    // For now, prefer creating our own floating window using X11 (as an example)
    // This choice can be platform-dependent.
    *api = CLAP_WINDOW_API_X11; // Or WIN32 on Windows, COCOA on macOS
    *is_floating = true;
    // *is_floating = false; // If we prefer embedding
    return true;
}

static bool gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: gui_create called (api: %s, is_floating: %s)\n", api, is_floating ? "true" : "false");

    if (!self->imgui_context) {
        fprintf(stderr, "MyPlugin GUI: ImGui context not initialized before gui_create!\n");
        return false;
    }

    // We only handle floating windows for now, created by us.
    // Or if the host suggests an API we know we can make a window with.
    if (is_floating || strcmp(api, CLAP_WINDOW_API_X11) == 0 || strcmp(api, CLAP_WINDOW_API_WIN32) == 0 || strcmp(api, CLAP_WINDOW_API_COCOA) == 0) {
        if (self->window) {
            printf("MyPlugin GUI: Window already exists. Reusing.\n");
            // Potentially bring to front if already created, or handle as an error/warning
            return true;
        }

        printf("MyPlugin GUI: Creating floating window...\n");
        if (!glfwInit()) {
            fprintf(stderr, "MyPlugin GUI: Failed to initialize GLFW in gui_create\n");
            return false;
        }

        self->window = glfwCreateWindow(800, 600, "CLAP ImGui Window (via gui_create)", NULL, NULL);
        if (!self->window) {
            fprintf(stderr, "MyPlugin GUI: Failed to create GLFW window in gui_create\n");
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(self->window);

        ImGui::SetCurrentContext(self->imgui_context); // Ensure ImGui context is current for these init calls

        if (!ImGui_ImplGlfw_InitForOpenGL(self->window, true)) {
            fprintf(stderr, "MyPlugin GUI: Failed to initialize ImGui GLFW backend in gui_create\n");
            glfwDestroyWindow(self->window);
            self->window = nullptr;
            glfwTerminate();
            return false;
        }
        if (!ImGui_ImplOpenGL3_Init("#version 130")) {
            fprintf(stderr, "MyPlugin GUI: Failed to initialize ImGui OpenGL3 backend in gui_create\n");
            ImGui_ImplGlfw_Shutdown();
            glfwDestroyWindow(self->window);
            self->window = nullptr;
            glfwTerminate();
            return false;
        }
        printf("MyPlugin GUI: Floating window created and ImGui backends initialized successfully.\n");
        return true;
    } else {
        // Logic for embedding into a parent window provided by `gui_set_parent`
        printf("MyPlugin GUI: Embedded window creation requested (api: %s). Not yet fully implemented.\n", api);
        return false; // Proper embedding not yet implemented
    }
}

static void gui_destroy(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: gui_destroy called\n");

    if (self->window) {
        printf("MyPlugin GUI: Destroying window and ImGui backends.\n");
        ImGui::SetCurrentContext(self->imgui_context); // Important for shutdown
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        glfwDestroyWindow(self->window);
        self->window = nullptr;
        glfwTerminate(); // Terminate GLFW as we initialized it
        printf("MyPlugin GUI: Window and ImGui backends destroyed.\n");
    } else {
        printf("MyPlugin GUI: No window to destroy.\n");
    }
    self->is_gui_visible = false; // Ensure GUI is marked as not visible
}

static bool gui_set_scale(const clap_plugin_t *plugin, double scale) {
    printf("MyPlugin GUI: gui_set_scale called (scale: %f)\n", scale);
    // You might want to pass this to ImGui IO: ImGui::GetIO().FontGlobalScale = scale;
    // Or handle HiDPI scaling if needed.
    return true;
}

static bool gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: gui_get_size called\n");
    if (self->window) {
        int w, h;
        glfwGetWindowSize(self->window, &w, &h);
        *width = (uint32_t)w;
        *height = (uint32_t)h;
    } else {
        // Default size if window not created yet or for headless mode
        *width = 800;
        *height = 600;
    }
    printf("MyPlugin GUI: current size is %u x %u\n", *width, *height);
    return true;
}

static bool gui_can_resize(const clap_plugin_t *plugin) {
    printf("MyPlugin GUI: gui_can_resize called\n");
    return true; // Our ImGui window (if floating) can be resized
}

static bool gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints) {
    printf("MyPlugin GUI: gui_get_resize_hints called\n");
    // No specific hints for now
    hints->can_resize_horizontally = true;
    hints->can_resize_vertically = true;
    hints->aspect_ratio_width = 0; // No fixed aspect ratio
    hints->aspect_ratio_height = 0;
    return true;
}

static bool gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    printf("MyPlugin GUI: gui_adjust_size called (requested: %u x %u)\n", *width, *height);
    // We accept any size for now
    // Could add min/max constraints here
    printf("MyPlugin GUI: adjusted size is %u x %u\n", *width, *height);
    return true;
}

static bool gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: gui_set_size called (new size: %u x %u)\n", width, height);
    if (self->window) {
        glfwSetWindowSize(self->window, (int)width, (int)height);
        return true;
    }
    return false; // No window to resize
}

static bool gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    printf("MyPlugin GUI: gui_set_parent called\n");
    if (window) {
        printf("MyPlugin GUI: Parent window details - API: %s\n", window->api);
        // Store this window handle if you plan to embed.
        // For X11, window->x11 contains the XID.
        // For Win32, window->win32 contains the HWND.
        // For Cocoa, window->cocoa contains the NSView*.
        // The ImGui GLFW backend might need to be re-initialized or a different backend used for embedding.
        // This example primarily focuses on a floating GLFW window.
    } else {
        printf("MyPlugin GUI: Parent window is null (likely detaching).\n");
    }
    return true;
}

static bool gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window) {
    printf("MyPlugin GUI: gui_set_transient called\n");
    if (window) {
        printf("MyPlugin GUI: Transient window details - API: %s\n", window->api);
    }
    return true;
}

static void gui_suggest_title(const clap_plugin_t *plugin, const char *title) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: gui_suggest_title called (title: %s)\n", title);
    if (self->window) {
        glfwSetWindowTitle(self->window, title);
    }
}

static bool gui_show(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: gui_show called\n");
    if (self->window) {
        glfwShowWindow(self->window);
        self->is_gui_visible = true;
        printf("MyPlugin GUI: Window shown, is_gui_visible = true\n");
        return true;
    }
    printf("MyPlugin GUI: No window to show.\n");
    return false;
}

static bool gui_hide(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: gui_hide called\n");
    if (self->window) {
        glfwHideWindow(self->window);
        self->is_gui_visible = false;
        printf("MyPlugin GUI: Window hidden, is_gui_visible = false\n");
        return true;
    }
    printf("MyPlugin GUI: No window to hide.\n");
    return false;
}


// --- Plugin Implementation ---
static bool my_plugin_init(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Initializing plugin\n");

    // Initialize CLAP GUI extension pointers
    self->plugin_gui_extension.is_api_supported = gui_is_api_supported;
    self->plugin_gui_extension.get_preferred_api = gui_get_preferred_api;
    self->plugin_gui_extension.create = gui_create;
    self->plugin_gui_extension.destroy = gui_destroy;
    self->plugin_gui_extension.set_scale = gui_set_scale;
    self->plugin_gui_extension.get_size = gui_get_size;
    self->plugin_gui_extension.can_resize = gui_can_resize;
    self->plugin_gui_extension.get_resize_hints = gui_get_resize_hints;
    self->plugin_gui_extension.adjust_size = gui_adjust_size;
    self->plugin_gui_extension.set_size = gui_set_size;
    self->plugin_gui_extension.set_parent = gui_set_parent;
    self->plugin_gui_extension.set_transient = gui_set_transient;
    self->plugin_gui_extension.suggest_title = gui_suggest_title;
    self->plugin_gui_extension.show = gui_show;
    self->plugin_gui_extension.hide = gui_hide;
    // self->is_gui_visible is initialized in my_factory_create_plugin

    // Initialize ImGui Core context (once per plugin instance)
    IMGUI_CHECKVERSION();
    self->imgui_context = ImGui::CreateContext();
    if (!self->imgui_context) {
        fprintf(stderr, "MyPlugin: Failed to create ImGui context\n");
        return false;
    }
    ImGui::SetCurrentContext(self->imgui_context);
    ImGui::StyleColorsDark();
    printf("MyPlugin: ImGui context initialized.\n");

    // GLFW window and ImGui backends are now initialized in gui_create
    // Initialize your plugin state here (e.g. parameters)
    return true;
}

static void my_plugin_destroy(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Destroying plugin\n");

    // ImGui backends and GLFW window are destroyed in gui_destroy
    // Destroy ImGui Core context (once per plugin instance)
    if (self->imgui_context) {
        ImGui::DestroyContext(self->imgui_context);
        self->imgui_context = nullptr;
        printf("MyPlugin: ImGui context destroyed.\n");
    }

    // Free any other plugin-specific resources allocated in init
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
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (strcmp(id, CLAP_EXT_GUI) == 0) {
        return &self->plugin_gui_extension;
    }
    // Example: if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) return &my_audio_ports_extension;
    // Example: if (strcmp(id, CLAP_EXT_PARAMS) == 0) return &my_params_extension;
    printf("MyPlugin: Host requesting extension: %s\n", id);
    return NULL;
}

static void my_plugin_on_main_thread(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;

    // Ensure ImGui and GLFW window are initialized AND GUI is visible
    if (!self || !self->imgui_context || !self->window || !self->is_gui_visible) {
        // If GUI is not visible, or not initialized, do nothing for rendering.
        // We might still want to process other main thread tasks if any.
        // printf("MyPlugin: on_main_thread - GUI not visible or not initialized.\n");
        return;
    }

    // It's crucial to set the correct context before calling ImGui or GLFW functions.
    ImGui::SetCurrentContext(self->imgui_context);
    glfwMakeContextCurrent(self->window); // Ensure GLFW context is current for this thread

    glfwPollEvents(); // Poll for and process events

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Show the ImGui demo window.
    // (In a real application, you would render your own GUI elements here)
    ImGui::ShowDemoWindow();

    // Rendering
    ImGui::Render();

    int display_w, display_h;
    glfwGetFramebufferSize(self->window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    // Clear color (example: dark gray)
    // Note: In a real plugin, you might want this to be transparent or match the host.
    // For a standalone window, this is fine.
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(self->window); // Swap buffers
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
    self->is_gui_visible = false; // Initialize GUI as not visible
    self->window = nullptr; // Initialize window pointer to null
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
