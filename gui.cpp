#include "gui.h"
#include "my_plugin.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __linux__
    #define GLFW_EXPOSE_NATIVE_X11
    #include <GLFW/glfw3native.h>
#elif _WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3native.h>
#endif

// Forward declarations
static bool gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating);
static bool gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating);
static bool gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating);
// gui_destroy is not static - it's declared in header
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

// The GUI extension interface
const clap_plugin_gui_t gui_extension = {
    .is_api_supported = gui_is_api_supported,
    .get_preferred_api = gui_get_preferred_api,
    .create = gui_create,
    .destroy = gui_destroy,
    .set_scale = gui_set_scale,
    .get_size = gui_get_size,
    .can_resize = gui_can_resize,
    .get_resize_hints = gui_get_resize_hints,
    .adjust_size = gui_adjust_size,
    .set_size = gui_set_size,
    .set_parent = gui_set_parent,
    .set_transient = gui_set_transient,
    .suggest_title = gui_suggest_title,
    .show = gui_show,
    .hide = gui_hide,
};

// Initialize GUI subsystem - call this once at plugin init
bool gui_init() {
    return glfwInit() == GLFW_TRUE;
}

void gui_cleanup() {
    glfwTerminate();
}

static bool gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    printf("GUI: Checking API support for %s (floating: %s)\n", api, is_floating ? "true" : "false");
    
    // Support the APIs based on platform
#ifdef __linux__
    if (strcmp(api, CLAP_WINDOW_API_X11) == 0) return true;
#elif _WIN32
    if (strcmp(api, CLAP_WINDOW_API_WIN32) == 0) return true;
#elif __APPLE__
    if (strcmp(api, CLAP_WINDOW_API_COCOA) == 0) return true;
#endif
    
    // We always support floating windows regardless of API
    if (is_floating) return true;
    
    return false;
}

static bool gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating) {
    printf("GUI: Host asking for preferred API\n");
    
    // Prefer embedded over floating
    *is_floating = false;
    
#ifdef __linux__
    *api = CLAP_WINDOW_API_X11;
#elif _WIN32
    *api = CLAP_WINDOW_API_WIN32;
#elif __APPLE__
    *api = CLAP_WINDOW_API_COCOA;
#else
    *is_floating = true;
    *api = "";
#endif
    
    return true;
}

static bool gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    printf("GUI: Creating GUI with API %s (floating: %s)\n", api ? api : "none", is_floating ? "true" : "false");
    
    if (self->gui.is_created) {
        printf("GUI: GUI already created\n");
        return false;
    }
    
    // Initialize our GUI structure
    self->gui.current_api = api;
    self->gui.is_floating = is_floating;
    self->gui.width = 400;  // Default size
    self->gui.height = 300;
    self->gui.is_created = false;
    self->gui.is_visible = false;
    self->gui.scale_factor = 1.0;
    self->gui.glfw_window = NULL;
    
    // Create GLFW window for rendering
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Start hidden
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    GLFWwindow *window = glfwCreateWindow(self->gui.width, self->gui.height, "CLAP Jules Plugin", NULL, NULL);
    if (!window) {
        printf("GUI: Failed to create GLFW window\n");
        return false;
    }
    
    self->gui.glfw_window = window;
    self->gui.is_created = true;
    
    printf("GUI: GUI created successfully\n");
    return true;
}

void gui_destroy(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    printf("GUI: Destroying GUI\n");
    
    if (!self->gui.is_created) {
        return;
    }
    
    if (self->gui.glfw_window) {
        glfwDestroyWindow((GLFWwindow*)self->gui.glfw_window);
        self->gui.glfw_window = NULL;
    }
    
    self->gui.is_created = false;
    self->gui.is_visible = false;
}

static bool gui_set_scale(const clap_plugin_t *plugin, double scale) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    printf("GUI: Setting scale to %.2f\n", scale);
    self->gui.scale_factor = scale;
    return true;
}

static bool gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    if (!self->gui.is_created) {
        return false;
    }
    
    *width = self->gui.width;
    *height = self->gui.height;
    
    printf("GUI: Returning size %dx%d\n", *width, *height);
    return true;
}

static bool gui_can_resize(const clap_plugin_t *plugin) {
    printf("GUI: Can resize: true\n");
    return true; // We support resizing
}

static bool gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints) {
    printf("GUI: Providing resize hints\n");
    
    hints->can_resize_horizontally = true;
    hints->can_resize_vertically = true;
    hints->preserve_aspect_ratio = false;
    hints->aspect_ratio_width = 4;
    hints->aspect_ratio_height = 3;
    
    return true;
}

static bool gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    printf("GUI: Adjusting size from %dx%d", *width, *height);
    
    // Ensure minimum size
    if (*width < 200) *width = 200;
    if (*height < 150) *height = 150;
    
    // Ensure maximum size
    if (*width > 1200) *width = 1200;
    if (*height > 900) *height = 900;
    
    printf(" to %dx%d\n", *width, *height);
    return true;
}

static bool gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    printf("GUI: Setting size to %dx%d\n", width, height);
    
    if (!self->gui.is_created) {
        return false;
    }
    
    self->gui.width = width;
    self->gui.height = height;
    
    if (self->gui.glfw_window) {
        glfwSetWindowSize((GLFWwindow*)self->gui.glfw_window, width, height);
    }
    
    return true;
}

static bool gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    printf("GUI: Setting parent window (API: %s)\n", window->api);
    
    if (!self->gui.is_created) {
        return false;
    }
    
    // Store the parent window information
    if (strcmp(window->api, CLAP_WINDOW_API_X11) == 0) {
        self->gui.window_handle.x11_window = window->x11;
        printf("GUI: Parent X11 window ID: %lu\n", window->x11);
    } else if (strcmp(window->api, CLAP_WINDOW_API_WIN32) == 0) {
        self->gui.window_handle.win32_hwnd = window->win32;
        printf("GUI: Parent Win32 HWND: %p\n", window->win32);
    } else if (strcmp(window->api, CLAP_WINDOW_API_COCOA) == 0) {
        self->gui.window_handle.cocoa_nsview = window->cocoa;
        printf("GUI: Parent Cocoa NSView: %p\n", window->cocoa);
    }
    
    return gui_set_parent_platform(self, window);
}

static bool gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window) {
    printf("GUI: Setting transient parent (floating window)\n");
    // For floating windows - make this window stay above the parent
    return true;
}

static void gui_suggest_title(const clap_plugin_t *plugin, const char *title) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    printf("GUI: Suggested title: %s\n", title);
    
    if (self->gui.glfw_window) {
        glfwSetWindowTitle((GLFWwindow*)self->gui.glfw_window, title);
    }
}

static bool gui_show(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    printf("GUI: Showing window\n");
    
    if (!self->gui.is_created) {
        return false;
    }
    
    if (self->gui.glfw_window) {
        glfwShowWindow((GLFWwindow*)self->gui.glfw_window);
    }
    
    self->gui.is_visible = true;
    return gui_show_platform(self);
}

static bool gui_hide(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    
    printf("GUI: Hiding window\n");
    
    if (!self->gui.is_created) {
        return false;
    }
    
    if (self->gui.glfw_window) {
        glfwHideWindow((GLFWwindow*)self->gui.glfw_window);
    }
    
    self->gui.is_visible = false;
    return gui_hide_platform(self);
}

// Platform-specific implementations
bool gui_create_platform_window(my_plugin_t *plugin, const char *api, bool is_floating) {
    // Platform-specific window creation if needed
    return true;
}

void gui_destroy_platform_window(my_plugin_t *plugin) {
    // Platform-specific cleanup
}

bool gui_set_parent_platform(my_plugin_t *plugin, const clap_window_t *window) {
#ifdef __linux__
    if (strcmp(window->api, CLAP_WINDOW_API_X11) == 0) {
        // TODO: Implement X11 window embedding using XEmbed
        printf("GUI: X11 embedding not yet implemented\n");
        return true;
    }
#elif _WIN32
    if (strcmp(window->api, CLAP_WINDOW_API_WIN32) == 0) {
        // TODO: Implement Win32 window embedding using SetParent
        printf("GUI: Win32 embedding not yet implemented\n");
        return true;
    }
#endif
    return false;
}

bool gui_show_platform(my_plugin_t *plugin) {
    return true;
}

bool gui_hide_platform(my_plugin_t *plugin) {
    return true;
}