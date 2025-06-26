// Platform-specific includes that define types used in my_plugin.h
// must come before my_plugin.h
#ifdef HAVE_GLFW
    // Platform-specific includes must come before GLFW native header
    #ifdef _WIN32
        #ifndef WIN32_LEAN_AND_MEAN
            #define WIN32_LEAN_AND_MEAN
        #endif
        #include <windows.h>
        #define GLFW_EXPOSE_NATIVE_WIN32
    #endif

    #ifdef __linux__
        #include <X11/Xlib.h>
        #include <X11/Xutil.h> // For XQueryPointer if not covered by Xlib.h alone for DefaultRootWindow
        #define GLFW_EXPOSE_NATIVE_X11
    #endif
#endif // HAVE_GLFW

#include "my_plugin.h"
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc

#ifdef HAVE_GLFW
#include <clap/ext/gui.h>

// GLFW includes (after platform headers and my_plugin.h if it depends on them)
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// OpenGL for basic rendering
#ifdef __APPLE__
#include <OpenGL/gl.h>
#elif defined(_WIN32)
#include <GL/gl.h>
#elif defined(__linux__)
#include <GL/gl.h>
#endif
#endif // HAVE_GLFW

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
// --- Forward declarations of GUI functions ---
static void my_plugin_gui_render(const clap_plugin_t *plugin);
static void my_plugin_gui_window_refresh_callback(GLFWwindow* window);
static void small_window_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
static void small_window_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
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
#endif // HAVE_GLFW

// --- Plugin Descriptor ---
// Features array for the plugin descriptor
static const char *const plugin_features[] = {"audio-effect", nullptr}; // Corrected to "audio-effect"

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
    
#ifdef HAVE_GLFW
    // Initialize GUI-related fields
    self->window = NULL;
    self->gui_created = false;
    self->gui_visible = false;
    self->gui_width = 400;
    self->gui_height = 300;
    self->gui_api = NULL;
    self->is_floating = false;
    self->needs_refresh = false;
    self->small_window = NULL; // Initialize small window
    self->small_window_x = 50;  // Initial position for small window
    self->small_window_y = 50;
    self->is_dragging_small_window = false;
    self->drag_offset_x = 0;
    self->drag_offset_y = 0;
    
    // Initialize GLFW if not already done
    if (!glfwInit()) {
        const char* error_desc;
        int error_code = glfwGetError(&error_desc);
        printf("MyPlugin: Failed to initialize GLFW - Error %d: %s\n", 
               error_code, error_desc ? error_desc : "Unknown error");
        // Don't fail plugin initialization if GLFW fails - just disable GUI
        self->gui_created = false;
        self->gui_visible = false;
        return true; // Plugin can still work without GUI
    }
#endif
    
    return true;
}

static void my_plugin_destroy(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Destroying plugin\n");
    
#ifdef HAVE_GLFW
    // Clean up GUI resources
    if (self->gui_created) {
        my_plugin_gui_destroy(plugin);
    }
    
    // Terminate GLFW
    glfwTerminate();
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

#ifdef HAVE_GLFW
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
#endif // HAVE_GLFW

static const void *my_plugin_get_extension(const struct clap_plugin *plugin, const char *id) {
#ifdef HAVE_GLFW
    // Return GUI extension if requested
    if (strcmp(id, CLAP_EXT_GUI) == 0) {
        return &my_plugin_gui_extension;
    }
#endif
    
    // Example: if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) return &my_audio_ports_extension;
    // Example: if (strcmp(id, CLAP_EXT_PARAMS) == 0) return &my_params_extension;
    printf("MyPlugin: Host requesting extension: %s\n", id);
    return NULL; // Extension not supported
}

static void my_plugin_on_main_thread(const struct clap_plugin *plugin) {
    // Called by the host to perform tasks that must run on the main thread.
#ifdef HAVE_GLFW
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    // Only poll events if GUI is created and visible.
    if (self->gui_created && self->gui_visible) {
        // Poll events globally. This should cover all GLFW windows.
        glfwPollEvents();
        
        // If a refresh is needed for the main plugin GUI, render everything.
        // The my_plugin_gui_render function handles drawing both the main window
        // and the small window if it exists.
        if (self->needs_refresh) {
            my_plugin_gui_render(plugin);
            self->needs_refresh = false;
        }
        // Note: The small window currently does not have its own independent refresh callback
        // or 'needs_refresh' flag. Its visual updates (beyond dragging) are tied to the
        // main window's refresh cycle or will happen during the next main window refresh.
        // If the small window required more frequent or independent updates,
        // it would need its own refresh mechanism (e.g., its own refresh callback setting a flag,
        // or direct rendering calls if certain events occur). For now, dragging updates its position,
        // and its content is redrawn when the main GUI is redrawn.
    }
#endif
}

#ifdef HAVE_GLFW
// --- GUI Extension Implementation ---

// GLFW window refresh callback for handling window redraws
static void my_plugin_gui_window_refresh_callback(GLFWwindow* window) {
    // Get plugin instance from window user pointer
    my_plugin_t* self = (my_plugin_t*)glfwGetWindowUserPointer(window);
    if (self && self->gui_created && self->gui_visible) {
        // Mark that we need a refresh and render immediately
        self->needs_refresh = true;
        my_plugin_gui_render(&self->plugin);
    }
}

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
    
    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("MyPlugin: OpenGL error before rendering: %d\n", error);
    }
    
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
    
    // Check for OpenGL errors
    error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("MyPlugin: OpenGL error after rendering: %d\n", error);
    }
    
    // Swap buffers to display the rendered frame
    glfwSwapBuffers(self->window);

    // Render the small window if it exists
    if (self->small_window) {
        glfwMakeContextCurrent(self->small_window);
        int small_width, small_height;
        glfwGetFramebufferSize(self->small_window, &small_width, &small_height);
        glViewport(0, 0, small_width, small_height);

        // Clear small window with a different color
        glClearColor(0.8f, 0.7f, 0.3f, 1.0f); // Yellowish
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw a simple border or something to indicate it's a different window
        glColor3f(0.1f, 0.1f, 0.1f); // Dark gray
        glBegin(GL_LINE_LOOP);
        glVertex2f(-0.95f, -0.95f);
        glVertex2f(0.95f, -0.95f);
        glVertex2f(0.95f, 0.95f);
        glVertex2f(-0.95f, 0.95f);
        glEnd();

        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            printf("MyPlugin: OpenGL error after rendering small window: %d\n", error);
        }

        glfwSwapBuffers(self->small_window);
    }
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
    
    if (!api) {
        printf("MyPlugin: No API specified\n");
        return false;
    }
    
    // Store GUI configuration
    self->gui_api = api;
    self->is_floating = is_floating;
    
    // Configure GLFW window hints
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Start hidden
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // Add conditional decoration hint
    if (self->is_floating == false) {
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    } else {
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    }
    
    // Create GLFW window
    self->window = glfwCreateWindow(
        self->gui_width, 
        self->gui_height, 
        "My CLAP Plugin", 
        NULL, 
        NULL
    );
    
    if (!self->window) {
        const char* error_desc;
        int error_code = glfwGetError(&error_desc);
        printf("MyPlugin: Failed to create GLFW window - Error %d: %s\n", 
               error_code, error_desc ? error_desc : "Unknown error");
        return false;
    }
    
    // Make context current for OpenGL operations
    glfwMakeContextCurrent(self->window);
    
    // Set up window callbacks for proper refresh handling
    glfwSetWindowUserPointer(self->window, self);
    glfwSetWindowRefreshCallback(self->window, my_plugin_gui_window_refresh_callback);
    
    // Check if we can get OpenGL context
    const char* gl_version = (const char*)glGetString(GL_VERSION);
    if (gl_version) {
        printf("MyPlugin: OpenGL version: %s\n", gl_version);
    } else {
        printf("MyPlugin: Warning - Could not get OpenGL version\n");
    }
    
    // Enable V-Sync
    glfwSwapInterval(1);
    
    // Set up basic OpenGL state
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f); // Dark blue background
    
    // Create the small, undecorated window
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    self->small_window = glfwCreateWindow(150, 100, "Small Window", NULL, self->window /* share context with main window */);
    if (!self->small_window) {
        const char* error_desc;
        int error_code = glfwGetError(&error_desc);
        printf("MyPlugin: Failed to create small GLFW window - Error %d: %s\n",
               error_code, error_desc ? error_desc : "Unknown error");
        // Continue without the small window if creation fails
    } else {
        glfwSetWindowUserPointer(self->small_window, self);
        glfwSetMouseButtonCallback(self->small_window, small_window_mouse_button_callback);
        glfwSetCursorPosCallback(self->small_window, small_window_cursor_pos_callback);
        // Initial position before parenting, will be relative to screen.
        // After parenting, this position might need to be 0,0 or relative to parent.
        glfwSetWindowPos(self->small_window, self->small_window_x, self->small_window_y);
        printf("MyPlugin: Small GUI window created successfully (pre-parenting)\n");

        // --- Native Parenting Logic ---
#ifdef _WIN32
        HWND main_hwnd = glfwGetWin32Window(self->window);
        HWND small_hwnd = glfwGetWin32Window(self->small_window);
        if (main_hwnd && small_hwnd) {
            self->small_hwnd = small_hwnd; // Store native handle
            printf("MyPlugin: Attempting Win32 SetParent...\n");
            SetParent(small_hwnd, main_hwnd);
            // Adjust styles for a child window
            LONG_PTR style = GetWindowLongPtr(small_hwnd, GWL_STYLE);
            style = style & ~WS_POPUP; // Remove popup style
            style = style | WS_CHILD;   // Add child style
            SetWindowLongPtr(small_hwnd, GWL_STYLE, style);

            // Set position relative to parent (e.g., 0,0 or self->small_window_x, self->small_window_y)
            // SetWindowPos is more robust for this. SWP_NOSIZE | SWP_NOZORDER
            SetWindowPos(small_hwnd, NULL, self->small_window_x, self->small_window_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
            printf("MyPlugin: Win32 SetParent and style adjustment attempted.\n");
        } else {
            printf("MyPlugin: Failed to get HWND for native parenting.\n");
        }
#elif __linux__
        Display* display = glfwGetX11Display();
        Window main_x11_window = glfwGetX11Window(self->window);
        Window small_x11_window = glfwGetX11Window(self->small_window);
        if (display && main_x11_window && small_x11_window) {
            self->x11_display = display; // Store X11 display
            self->small_x11_window = small_x11_window; // Store native handle
            printf("MyPlugin: Attempting X11 ReparentWindow...\n");
            XReparentWindow(display, small_x11_window, main_x11_window, self->small_window_x, self->small_window_y);
            XMapWindow(display, small_x11_window); // Ensure it's mapped after reparenting
            XFlush(display);
            printf("MyPlugin: X11 ReparentWindow attempted.\n");
        } else {
            printf("MyPlugin: Failed to get X11 handles for native parenting.\n");
        }
#endif
    }
    // Restore decoration hint for any subsequent windows
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

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
    if (self->small_window) { // Destroy the small window
        glfwDestroyWindow(self->small_window);
        self->small_window = NULL;
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
    
    printf("MyPlugin: GUI set parent - API: %s\n", window ? window->api : "NULL");
    
    if (!self->gui_created || !self->window) {
        printf("MyPlugin: GUI not created, cannot set parent\n");
        return false;
    }
    
    if (!window || !window->api) {
        printf("MyPlugin: Invalid window parameter\n");
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
            printf("MyPlugin: Failed to get X11 handles (display=%p, child=%lu, parent=%lu)\n", 
                   display, child, parent);
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
            if (SetParent(child, parent) != NULL) {
                printf("MyPlugin: Win32 window reparented successfully\n");
                return true;
            } else {
                DWORD error = GetLastError();
                printf("MyPlugin: SetParent failed with error %lu\n", error);
                return false;
            }
        } else {
            printf("MyPlugin: Failed to get Win32 handles (child=%p, parent=%p)\n", child, parent);
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
    if (self->small_window) {
        glfwShowWindow(self->small_window);
    }
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
    if (self->small_window) {
        glfwHideWindow(self->small_window);
    }
    self->gui_visible = false;
    
    return true;
}

// --- Helper function for getting screen cursor position ---
static bool my_plugin_get_screen_cursor_position(my_plugin_t* self, int* x, int* y) {
    if (!x || !y) return false;

#ifdef _WIN32
    POINT cursorPos;
    if (GetCursorPos(&cursorPos)) {
        *x = cursorPos.x;
        *y = cursorPos.y;
        return true;
    } else {
        printf("MyPlugin: GetCursorPos failed.\n");
        return false;
    }
#elif __linux__
    if (self && self->x11_display) { // Check self for x11_display
        Window root_return, child_return;
        int root_x_return, root_y_return, win_x_return, win_y_return;
        unsigned int mask_return;
        if (XQueryPointer(self->x11_display, DefaultRootWindow(self->x11_display),
                          &root_return, &child_return,
                          &root_x_return, &root_y_return,
                          &win_x_return, &win_y_return, &mask_return)) {
            *x = root_x_return;
            *y = root_y_return;
            return true;
        } else {
            printf("MyPlugin: XQueryPointer failed.\n");
            return false;
        }
    } else {
        printf("MyPlugin: X11 display not available for screen cursor query.\n");
        return false;
    }
#else
    printf("MyPlugin: Platform not supported for native screen cursor position helper.\n");
    // Fallback: try to use GLFW's understanding if possible, though less ideal.
    // This would require the GLFWwindow* context, which this helper doesn't have.
    // For now, just indicate failure for unsupported platforms.
    return false;
#endif
    return false; // Ensure all paths return a value, e.g. if not WIN32 or __linux__
}

// --- Mouse Callback Functions for Small Window ---
static void small_window_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    my_plugin_t* self = (my_plugin_t*)glfwGetWindowUserPointer(window);
    if (!self) return;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            self->is_dragging_small_window = true;

            // Store initial small window position (which should be parent-relative)
            self->initial_small_window_relative_x = self->small_window_x;
            self->initial_small_window_relative_y = self->small_window_y;

            // Get and store initial mouse cursor screen coordinates using the helper
            if (!my_plugin_get_screen_cursor_position(self, &self->initial_drag_screen_x, &self->initial_drag_screen_y)) {
                self->is_dragging_small_window = false; // Cancel drag
                printf("MyPlugin: Failed to get initial screen cursor position via helper.\n");
                // Potentially use fallback if defined, or just return.
                // For now, if the helper fails, we cancel the drag.
                // This could happen if X11 display isn't set yet, or GetCursorPos fails.
                return;
            }

            // The old drag_offset_x/y were relative to the small window's client area.
            // For the new logic, we primarily use screen coordinates for delta calculation
            // and apply it to the initial parent-relative position.
            // So, drag_offset_x/y might not be strictly needed in the new approach
            // unless we want to maintain the exact click point within the window as the drag handle.
            // For now, we'll rely on the screen coordinate delta.
            printf("MyPlugin: Small window drag started. Initial screen cursor: (%d, %d), initial window relative: (%d, %d)\n",
                   self->initial_drag_screen_x, self->initial_drag_screen_y,
                   self->initial_small_window_relative_x, self->initial_small_window_relative_y);

        } else if (action == GLFW_RELEASE) {
            self->is_dragging_small_window = false;
            printf("MyPlugin: Small window drag ended\n");
        }
    }
}

static void small_window_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    my_plugin_t* self = (my_plugin_t*)glfwGetWindowUserPointer(window);
    if (!self || !self->is_dragging_small_window) return;

    // `xpos` and `ypos` from GLFW are relative to the client area of the small window.
    // We need current screen cursor position for our new logic.

    // `xpos` and `ypos` from GLFW are relative to the client area of the small window.
    // We need current screen cursor position for our new logic.

    int current_screen_cursor_x = 0;
    int current_screen_cursor_y = 0;

    if (!my_plugin_get_screen_cursor_position(self, &current_screen_cursor_x, &current_screen_cursor_y)) {
        printf("MyPlugin: Failed to get current screen cursor position via helper during drag.\n");
        // If we can't get the current cursor position, we can't calculate the delta.
        // Options:
        // 1. Stop the drag (self->is_dragging_small_window = false;)
        // 2. Skip this update and hope the next one works.
        // 3. Fallback to old logic (problematic).
        // For now, just return and skip this update.
        return;
    }

    // Calculate delta movement in screen coordinates
    int delta_x = current_screen_cursor_x - self->initial_drag_screen_x;
    int delta_y = current_screen_cursor_y - self->initial_drag_screen_y;

    // Calculate new target parent-relative coordinates
    int new_relative_x = self->initial_small_window_relative_x + delta_x;
    int new_relative_y = self->initial_small_window_relative_y + delta_y;

    // Move the small window using native calls
#ifdef _WIN32
    if (self->small_hwnd) {
        SetWindowPos(self->small_hwnd, NULL, new_relative_x, new_relative_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
    } else {
        printf("MyPlugin: Small window HWND not available for SetWindowPos.\n");
        // Fallback or error
    }
#elif __linux__
    if (self->x11_display && self->small_x11_window) {
        XMoveWindow(self->x11_display, self->small_x11_window, new_relative_x, new_relative_y);
        XFlush(self->x11_display); // Ensure the move command is processed
    } else {
        printf("MyPlugin: X11 display or small window handle not available for XMoveWindow.\n");
        // Fallback or error
    }
#else
    // If native calls are not available (e.g. on other platforms, or if setup failed)
    // then the drag won't work with this new logic.
    // The earlier fallback in this function already tried to use old GLFW method.
    printf("MyPlugin: Native window move not supported on this platform. Drag will not work as intended.\n");
    // No operation here as the fallback was already attempted.
#endif

    // Update stored parent-relative position
    self->small_window_x = new_relative_x;
    self->small_window_y = new_relative_y;

    // No need to update drag_offset_x/y as they were for the old method.
    // The screen coordinates are absolute, and deltas are calculated from drag start.
}

#endif // HAVE_GLFW

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
