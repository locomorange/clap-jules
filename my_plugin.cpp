#include "my_plugin.h"
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp
#include <cstdlib>  // For calloc

#include "clap/ext/gui.h" // Include the CLAP GUI extension header
#include "imgui.h"      // Assuming imgui.h is accessible

// Define these before including imgui_impl_vulkan.h
// This would typically be configured by your build system.
// #define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h> // Must be included before imgui_impl_vulkan.h
#include "backends/imgui_impl_vulkan.h" // Adjusted path

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
    gui_is_api_supported, gui_get_preferred_api, gui_create, gui_destroy,
    gui_set_scale, gui_get_size, gui_can_resize, gui_get_resize_hints,
    gui_adjust_size, gui_set_size, gui_set_parent, gui_set_transient,
    gui_suggest_title, gui_show, gui_hide,
};

// --- Plugin Descriptor ---
static const char *const plugin_features[] = {"audio_effect", nullptr};
static const clap_plugin_descriptor_t my_plugin_descriptor = {
    CLAP_VERSION, "com.example.myplugin", "My First CLAP Plugin", "My Company",
    "https://example.com", "https://example.com/bugtracker", "https://example.com/support",
    "0.0.1", "A simple example CLAP audio plugin.", plugin_features,
};

// --- Plugin Implementation ---
static bool my_plugin_init(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Initializing plugin\n");
    self->imgui_context = ImGui::CreateContext();
    if (!self->imgui_context) {
        printf("MyPlugin: Warning - failed to create ImGui context\n");
        return false;
    }
    ImGui::SetCurrentContext(self->imgui_context);
    printf("MyPlugin: ImGui context created successfully (version: %s)\n", ImGui::GetVersion());
    self->vulkan_device = nullptr;
    return true;
}

static void my_plugin_destroy(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Destroying plugin\n");
    if (self->imgui_context) {
        // gui_destroy should have handled ImGui_ImplVulkan_Shutdown if Vulkan was active
        ImGui::DestroyContext(self->imgui_context);
        self->imgui_context = nullptr;
        printf("MyPlugin: ImGui context destroyed\n");
    }
}

static bool my_plugin_activate(const struct clap_plugin *plugin, double sr, uint32_t min_f, uint32_t max_f) { printf("MyPlugin: Activating\n"); return true; }
static void my_plugin_deactivate(const struct clap_plugin *plugin) { printf("MyPlugin: Deactivating\n"); }
static bool my_plugin_start_processing(const struct clap_plugin *plugin) { printf("MyPlugin: Start Processing\n"); return true; }
static void my_plugin_stop_processing(const struct clap_plugin *plugin) { printf("MyPlugin: Stop Processing\n"); }
static void my_plugin_reset(const struct clap_plugin *plugin) { printf("MyPlugin: Reset\n"); }
static clap_process_status my_plugin_process(const struct clap_plugin *plugin, const clap_process_t *process) { return CLAP_PROCESS_CONTINUE; }

static const void *my_plugin_get_extension(const struct clap_plugin *plugin, const char *id) {
    if (strcmp(id, CLAP_EXT_GUI) == 0) return &my_plugin_gui;
    printf("MyPlugin: Host requesting extension: %s\n", id);
    return NULL;
}

static void my_plugin_on_main_thread(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    if (!self->gui_created || !self->gui_is_visible || !self->imgui_context) return;

    ImGui::SetCurrentContext(self->imgui_context);
    ImGuiIO& io = ImGui::GetIO();
    // TODO: Update io.DisplaySize, io.DeltaTime, mouse input, etc. from host

    if (self->vulkan_device) {
        ImGui_ImplVulkan_NewFrame();
    } else {
        // printf("MyPlugin GUI: In on_main_thread, Vulkan backend not used/initialized. Placeholder for other backend NewFrame().\n");
    }

    ImGui::NewFrame();
    if (self->gui_is_visible) ImGui::ShowDemoWindow(&self->gui_is_visible);
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();

    if (self->vulkan_device && draw_data) {
        printf("MyPlugin GUI: Placeholder for Vulkan command buffer recording & submission (ImGui_ImplVulkan_RenderDrawData).\n");
    } else if (draw_data) {
        // printf("MyPlugin GUI: In on_main_thread, Vulkan backend not used/initialized during render. Placeholder for other backend RenderDrawData().\n");
    }
}

// --- GUI Function Implementations ---
static bool gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating) { /* ... as previously corrected ... */
    if (is_floating) return false;
#if defined(_WIN32)
    if (strcmp(api, CLAP_WINDOW_API_WIN32) == 0) return true;
#elif defined(__linux__) || defined(__FreeBSD__)
    if (strcmp(api, CLAP_WINDOW_API_X11) == 0) return true;
#endif
    return false;
}
static bool gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating) { /* ... as previously corrected ... */
    *is_floating = false;
#if defined(__linux__) || defined(__FreeBSD__)
    *api = CLAP_WINDOW_API_X11; return true;
#elif defined(_WIN32)
    *api = CLAP_WINDOW_API_WIN32; return true;
#endif
    return false;
}

static bool gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: Create (API: %s, Floating: %s)\n", api, is_floating ? "yes" : "no");
    self->vulkan_device = nullptr;

    if (!self->imgui_context) {
        printf("MyPlugin GUI: Error - ImGui context is null in gui_create.\n");
        return false;
    }
    ImGui::SetCurrentContext(self->imgui_context);

    bool should_attempt_vulkan = false;
#if defined(__linux__) || defined(__FreeBSD__)
    if (strcmp(api, CLAP_WINDOW_API_X11) == 0) should_attempt_vulkan = true;
#elif defined(_WIN32)
    if (strcmp(api, CLAP_WINDOW_API_WIN32) == 0) should_attempt_vulkan = true;
#endif

    if (should_attempt_vulkan) {
        printf("MyPlugin GUI: Attempting to initialize ImGui Vulkan backend for API %s.\n", api);
        // Placeholder for ImGui_ImplVulkan_Init and font creation
        // self->vulkan_device = actual_device_from_host_or_init; // This would be set on successful init
        self->vulkan_device = (VkDevice_T*)0x1; // Dummy non-null to indicate Vulkan path for now
        printf("MyPlugin GUI: Placeholder for ImGui_ImplVulkan_Init & CreateFontsTexture.\n");
    } else {
        printf("MyPlugin GUI: API %s not designated for Vulkan attempt or unsupported.\n", api);
    }
    self->gui_created = true;
    return true;
}

static void gui_destroy(const clap_plugin_t *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin GUI: Destroy\n");
    if (self->vulkan_device) {
        if (self->imgui_context) ImGui::SetCurrentContext(self->imgui_context);
        ImGui_ImplVulkan_Shutdown();
        printf("MyPlugin GUI: ImGui Vulkan backend shut down.\n");
        self->vulkan_device = nullptr;
    }
    self->gui_created = false;
}

static bool gui_set_scale(const clap_plugin_t *plugin, double scale) { printf("MyPlugin GUI: Set Scale (%.2f)\n", scale); return true; }
static bool gui_get_size(const clap_plugin_t *plugin, uint32_t *w, uint32_t *h) { *w = 400; *h = 300; return true; }
static bool gui_can_resize(const clap_plugin_t *plugin) { return false; }
static bool gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints) { return true; }
static bool gui_adjust_size(const clap_plugin_t *plugin, uint32_t *w, uint32_t *h) { *w = 400; *h = 300; return true; }
static bool gui_set_size(const clap_plugin_t *plugin, uint32_t w, uint32_t h) { return true; }
static bool gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) { ((my_plugin_t*)plugin->plugin_data)->parent_window = *window; return true; }
static bool gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window) { return true; }
static void gui_suggest_title(const clap_plugin_t *plugin, const char *title) {}
static bool gui_show(const clap_plugin_t *plugin) { ((my_plugin_t*)plugin->plugin_data)->gui_is_visible = true; return true; }
static bool gui_hide(const clap_plugin_t *plugin) { ((my_plugin_t*)plugin->plugin_data)->gui_is_visible = false; return true; }

// --- Plugin Factory Functions ---
static uint32_t my_factory_get_plugin_count(const struct clap_plugin_factory *factory) {
    return 1;
}

static const clap_plugin_descriptor_t *my_factory_get_plugin_descriptor(const struct clap_plugin_factory *factory, uint32_t index) {
    if (index == 0) return &my_plugin_descriptor;
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
    self->plugin.plugin_data = self;
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

// --- CLAP Entry Point ---
CLAP_EXPORT const struct clap_plugin_factory my_plugin_factory = {
    my_factory_get_plugin_count,
    my_factory_get_plugin_descriptor,
    my_factory_create_plugin,
};

CLAP_EXPORT const struct clap_plugin_factory* get_plugin_factory() { return &my_plugin_factory; }

CLAP_EXPORT const clap_plugin_entry_t clap_entry = {
    CLAP_VERSION,
    [](const char *path) -> bool { printf("MyPlugin: clap_entry.init(%s)\n",path); return true; },
    []() -> void { printf("MyPlugin: clap_entry.deinit()\n"); },
    [](const char *id) -> const void * {
        if (strcmp(id, CLAP_PLUGIN_FACTORY_ID) == 0) return &my_plugin_factory;
        return NULL;
    }
};
