#include "my_plugin.h"
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc
#include "clap/ext/gui.h" // Include the CLAP GUI extension header
#include "imgui.h"      // Assuming imgui.h is accessible

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

// --- Forward declarations for GUI functions ---
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

// --- GUI Struct ---
static const clap_plugin_gui_t my_plugin_gui = {
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
    
    // Initialize ImGui context for this plugin instance
    self->imgui_context = ImGui::CreateContext();
    if (!self->imgui_context) {
        printf("MyPlugin: Warning - failed to create ImGui context\n");
        return false;
    }
    
    // Set the context as current and perform basic setup
    ImGui::SetCurrentContext(self->imgui_context);
    printf("MyPlugin: ImGui context created successfully (version: %s)\n", ImGui::GetVersion());
    
    return true;
}

static void my_plugin_destroy(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Destroying plugin\n");
    
    // Clean up ImGui context
    if (self->imgui_context) {
        ImGui::SetCurrentContext(self->imgui_context);
        ImGui::DestroyContext(self->imgui_context);
        self->imgui_context = nullptr;
        printf("MyPlugin: ImGui context destroyed\n");
    }
    
    // Free any other resources allocated in init
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
    if (strcmp(id, CLAP_EXT_GUI) == 0) {
        return &my_plugin_gui;
    }
    // Example: if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) return &my_audio_ports_extension;
    // Example: if (strcmp(id, CLAP_EXT_PARAMS) == 0) return &my_params_extension;
    printf("MyPlugin: Host requesting extension: %s\n", id);
    return NULL;
}

static void my_plugin_on_main_thread(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;

    if (!self->gui_created || !self->gui_is_visible || !self->imgui_context) {
        // Don't do any rendering if GUI is not created, not visible, or context is missing
        return;
    }

    ImGui::SetCurrentContext(self->imgui_context);
    ImGuiIO& io = ImGui::GetIO();

    // TODO: Update display size if it can change (e.g., from host resizing the window)
    // For example:
    // uint32_t width, height;
    // if (gui_get_size(plugin, &width, &height)) { // Assuming gui_get_size can fetch current actual size
    //    io.DisplaySize = ImVec2((float)width, (float)height);
    // }
    
    // TODO: Call backend-specific NewFrame function (e.g., ImGui_ImplOpenGL3_NewFrame())
    // This needs to be set up in gui_create() based on the API and window handle.
    // For example, if using OpenGL: ImGui_ImplOpenGL3_NewFrame();
    // For example, if using a windowing library like GLFW (often with OpenGL): glfwPollEvents(); ImGui_ImplGlfw_NewFrame();
    // printf("MyPlugin GUI: Placeholder for backend NewFrame() call.\n");

    // Start the ImGui frame
    ImGui::NewFrame();

    // --- Render Plugin UI ---
    // For testing purposes, show the ImGui demo window
    if (self->gui_is_visible) { // Double check visibility, might be redundant but safe
        ImGui::ShowDemoWindow(&self->gui_is_visible); // Pass visibility to allow closing
        
        // Example of a simple custom window:
        // ImGui::Begin("My Plugin Window");
        // ImGui::Text("Hello from My CLAP Plugin!");
        // ImGui::End();
    }
    // --- End Render Plugin UI ---

    // Render ImGui draw data
    ImGui::Render();

    // TODO: Call backend-specific RenderDrawData function
    // (e.g., ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()))
    // This also needs to be set up in gui_create().
    // For example, if using OpenGL: glClearColor(0.0f, 0.0f, 0.0f, 1.0f); glClear(GL_COLOR_BUFFER_BIT); ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // printf("MyPlugin GUI: Placeholder for backend RenderDrawData() call.\n");
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

    // Initialize plugin data
    self->imgui_context = nullptr;

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

// Function to get the plugin factory (for cross-platform compatibility)
CLAP_EXPORT const struct clap_plugin_factory* get_plugin_factory() {
    return &my_plugin_factory;
}

// --- GUI Function Implementations ---
static bool gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    // For this example, we'll say we support the platform's native API for embedded GUIs.
    // And that we do not support floating GUIs.
    if (is_floating) return false;
#if defined(_WIN32)
    return strcmp(api, CLAP_WINDOW_API_WIN32) == 0;
#elif defined(__APPLE__)
    return strcmp(api, CLAP_WINDOW_API_COCOA) == 0;
#elif defined(__linux__) || defined(__FreeBSD__)
    return strcmp(api, CLAP_WINDOW_API_X11) == 0;
#else
    return false; // Unsupported platform
#endif
}

static bool gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating) {
    *is_floating = false; // We prefer embedded
#if defined(_WIN32)
    *api = CLAP_WINDOW_API_WIN32;
#elif defined(__APPLE__)
    *api = CLAP_WINDOW_API_COCOA;
#elif defined(__linux__) || defined(__FreeBSD__)
    *api = CLAP_WINDOW_API_X11;
#else
    return false; // No preferred API on unsupported platform
#endif
    return true;
}

static bool gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: Create (API: %s, Floating: %s)\n", api, is_floating ? "yes" : "no");

    // Initialize ImGui context if not already done (should be done in plugin_init)
    if (!self->imgui_context) {
        self->imgui_context = ImGui::CreateContext();
        ImGui::SetCurrentContext(self->imgui_context);
        // Perform basic ImGui setup if needed (e.g. styles, fonts)
        printf("MyPlugin GUI: ImGui context created in gui_create\n");
    } else {
        ImGui::SetCurrentContext(self->imgui_context);
        printf("MyPlugin GUI: Using existing ImGui context\n");
    }

    // TODO: Initialize ImGui rendering backend (e.g., ImGui_ImplOpenGL3_Init, ImGui_ImplDX11_Init)
    // This depends on the host's graphics context provided via gui_set_parent or similar.
    // For now, we'll assume this is handled elsewhere or by the host.
    printf("MyPlugin GUI: Placeholder for ImGui backend initialization.\n");

    // Example: Store the fact that GUI has been created
    self->gui_created = true;

    return true;
}

static void gui_destroy(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: Destroy\n");

    // TODO: Shutdown ImGui rendering backend (e.g., ImGui_ImplOpenGL3_Shutdown)
    printf("MyPlugin GUI: Placeholder for ImGui backend shutdown.\n");

    // The ImGui context itself is destroyed in my_plugin_destroy,
    // as it's tied to the plugin instance lifecycle, not just the GUI visibility.

    self->gui_created = false;
}

static bool gui_set_scale(const clap_plugin_t *plugin, double scale) {
    printf("MyPlugin GUI: Set Scale (Scale: %.2f)\n", scale);
    // TODO: Handle scaling if your GUI framework needs it. ImGui usually handles this via io.FontGlobalScale.
    // ImGuiIO& io = ImGui::GetIO();
    // io.FontGlobalScale = scale;
    return true;
}

static bool gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    // TODO: Return the actual size of your GUI.
    // For now, a fixed size.
    *width = 400;
    *height = 300;
    printf("MyPlugin GUI: Get Size (Width: %u, Height: %u)\n", *width, *height);
    return true;
}

static bool gui_can_resize(const clap_plugin_t *plugin) {
    printf("MyPlugin GUI: Can Resize\n");
    return false; // For this example, let's say the GUI is not resizable.
}

static bool gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints) {
    printf("MyPlugin GUI: Get Resize Hints\n");
    // Fill in hints if resizable. Since gui_can_resize is false, this might not be called.
    hints->can_resize_horizontally = false;
    hints->can_resize_vertically = false;
    hints->aspect_ratio_width = 0; // No fixed aspect ratio
    hints->aspect_ratio_height = 0;
    return true;
}

static bool gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    printf("MyPlugin GUI: Adjust Size (Requested Width: %u, Height: %u)\n", *width, *height);
    // If resizable, adjust the requested size to the closest valid size.
    // Since not resizable, we can just return the fixed size.
    *width = 400;
    *height = 300;
    return true;
}

static bool gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height) {
    printf("MyPlugin GUI: Set Size (Width: %u, Height: %u)\n", width, height);
    // Handle if the host forces a size. If not resizable, this might conflict.
    // For ImGui, the display size is usually updated by the backend.
    // ImGuiIO& io = ImGui::GetIO();
    // io.DisplaySize = ImVec2((float)width, (float)height);
    return true; // Return true if size was accepted.
}

static bool gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    printf("MyPlugin GUI: Set Parent (Window Handle: %p)\n", (void*)window->ptr);
    // This is where you would attach your GUI to the parent window.
    // For ImGui, the backend (e.g., ImGui_ImplOpenGL3_Init) often takes the window handle.
    // Store window->ptr if needed for backend initialization in gui_create or here.
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    self->parent_window = *window; // Store the window details
    return true;
}

static bool gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window) {
    printf("MyPlugin GUI: Set Transient (Window Handle: %p)\n", (void*)window->ptr);
    // For floating windows, set them as transient to this window if applicable.
    return true;
}

static void gui_suggest_title(const clap_plugin_t *plugin, const char *title) {
    printf("MyPlugin GUI: Suggest Title (%s)\n", title);
    // Store this title if you want to use it for a floating window.
}

static bool gui_show(const clap_plugin_t *plugin) {
    printf("MyPlugin GUI: Show\n");
    // Make the GUI visible.
    // For ImGui, rendering usually happens in a loop controlled by the host or plugin's audio thread.
    // This call might just set a flag.
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    self->gui_is_visible = true;
    return true;
}

static bool gui_hide(const clap_plugin_t *plugin) {
    printf("MyPlugin GUI: Hide\n");
    // Make the GUI invisible.
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    self->gui_is_visible = false;
    return true;
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
