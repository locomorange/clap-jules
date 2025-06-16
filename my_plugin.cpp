#include "my_plugin.h"
#include "graphics_wrapper.h"
#include <stdio.h>  // For printf in example functions
#include <string.h> // For strcmp and memset
#include <cstdlib>  // For calloc

// Platform-specific includes for windowing
#ifdef _WIN32
    #include <windows.h>
    #include <windowsx.h>
#elif defined(__APPLE__)
    #include <Cocoa/Cocoa.h>
#else
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <X11/Xatom.h>
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

// --- GUI Extension function declarations ---
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

// --- Platform-specific window management ---
static bool create_platform_window(gui_state_t *gui);
static void destroy_platform_window(gui_state_t *gui);
static void render_graphics(gui_state_t *gui);

#ifdef _WIN32
static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#elif defined(__linux__)
static void handle_x11_events(gui_state_t *gui);
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

// --- GUI Extension Structure ---
static const clap_plugin_gui_t gui_extension = {
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


// --- Plugin Implementation ---
static bool my_plugin_init(const struct clap_plugin *plugin) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    printf("MyPlugin: Initializing plugin\n");
    
    // Initialize GUI state
    memset(&self->gui, 0, sizeof(gui_state_t));
    self->gui.width = 400;
    self->gui.height = 300;
    self->gui.scale = 1.0;
    
    // Demonstrate Skia integration
    printf("MyPlugin: Skia support %s\n", 
           clap_jules::GraphicsContext::isSkiaAvailable() ? "enabled" : "disabled (using software fallback)");
    
    // Create a simple graphics test
    try {
        clap_jules::GraphicsContext graphics(100, 100);
        graphics.clear(0xFF1E1E1E);  // Dark gray background
        graphics.drawRect(10, 10, 80, 80, 0xFF4169E1);  // Royal blue rectangle
        graphics.drawCircle(50, 50, 20, 0xFFFFD700);    // Gold circle
        graphics.drawLine(0, 0, 99, 99, 0xFFFF0000);    // Red diagonal line
        
        printf("MyPlugin: Graphics test completed successfully\n");
    } catch (...) {
        printf("MyPlugin: Graphics test failed\n");
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
        return &gui_extension;
    }
    
    // Example: if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) return &my_audio_ports_extension;
    // Example: if (strcmp(id, CLAP_EXT_PARAMS) == 0) return &my_params_extension;
    return NULL; // Extension not supported
}

static void my_plugin_on_main_thread(const struct clap_plugin *plugin) {
    // Called by the host to perform tasks that must run on the main thread.
    // printf("MyPlugin: on_main_thread called\n");
}

// --- GUI Extension Implementation ---

static bool gui_is_api_supported(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    printf("MyPlugin: GUI - checking API support: %s (floating: %s)\n", api, is_floating ? "yes" : "no");
    
#ifdef _WIN32
    return strcmp(api, CLAP_WINDOW_API_WIN32) == 0;
#elif defined(__APPLE__)
    return strcmp(api, CLAP_WINDOW_API_COCOA) == 0;
#else
    return strcmp(api, CLAP_WINDOW_API_X11) == 0;
#endif
}

static bool gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *is_floating) {
    printf("MyPlugin: GUI - getting preferred API\n");
    
#ifdef _WIN32
    *api = CLAP_WINDOW_API_WIN32;
#elif defined(__APPLE__)
    *api = CLAP_WINDOW_API_COCOA;
#else
    *api = CLAP_WINDOW_API_X11;
#endif
    *is_floating = false; // Prefer embedded windows
    return true;
}

static bool gui_create(const clap_plugin_t *plugin, const char *api, bool is_floating) {
    printf("MyPlugin: GUI - creating GUI (API: %s, floating: %s)\n", api ? api : "null", is_floating ? "yes" : "no");
    
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    gui_state_t *gui = &self->gui;
    
    if (gui->is_created) {
        printf("MyPlugin: GUI - already created\n");
        return false;
    }
    
    gui->api = api;
    gui->is_floating = is_floating;
    
    // Create graphics context
    try {
        gui->graphics = new clap_jules::GraphicsContext(gui->width, gui->height);
        gui->is_created = true;
        printf("MyPlugin: GUI - created successfully\n");
        return true;
    } catch (...) {
        printf("MyPlugin: GUI - failed to create graphics context\n");
        return false;
    }
}

static void gui_destroy(const clap_plugin_t *plugin) {
    printf("MyPlugin: GUI - destroying GUI\n");
    
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    gui_state_t *gui = &self->gui;
    
    if (!gui->is_created) {
        return;
    }
    
    if (gui->is_visible) {
        gui_hide(plugin);
    }
    
    destroy_platform_window(gui);
    
    if (gui->graphics) {
        delete gui->graphics;
        gui->graphics = nullptr;
    }
    
    gui->is_created = false;
    printf("MyPlugin: GUI - destroyed\n");
}

static bool gui_set_scale(const clap_plugin_t *plugin, double scale) {
    printf("MyPlugin: GUI - setting scale: %.2f\n", scale);
    
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    gui_state_t *gui = &self->gui;
    
    gui->scale = scale;
    return true;
}

static bool gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    gui_state_t *gui = &self->gui;
    
    *width = gui->width;
    *height = gui->height;
    
    printf("MyPlugin: GUI - getting size: %dx%d\n", *width, *height);
    return true;
}

static bool gui_can_resize(const clap_plugin_t *plugin) {
    printf("MyPlugin: GUI - can resize: yes\n");
    return true; // Allow resizing
}

static bool gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints) {
    printf("MyPlugin: GUI - getting resize hints\n");
    
    hints->can_resize_horizontally = true;
    hints->can_resize_vertically = true;
    hints->preserve_aspect_ratio = false;
    hints->aspect_ratio_width = 0;
    hints->aspect_ratio_height = 0;
    
    return true;
}

static bool gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    printf("MyPlugin: GUI - adjusting size: %dx%d\n", *width, *height);
    
    // Set minimum size constraints
    if (*width < 200) *width = 200;
    if (*height < 150) *height = 150;
    
    // Set maximum size constraints
    if (*width > 1200) *width = 1200;
    if (*height > 800) *height = 800;
    
    printf("MyPlugin: GUI - adjusted size: %dx%d\n", *width, *height);
    return true;
}

static bool gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height) {
    printf("MyPlugin: GUI - setting size: %dx%d\n", width, height);
    
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    gui_state_t *gui = &self->gui;
    
    gui->width = width;
    gui->height = height;
    
    // Recreate graphics context with new size
    if (gui->graphics) {
        delete gui->graphics;
        try {
            gui->graphics = new clap_jules::GraphicsContext(width, height);
        } catch (...) {
            printf("MyPlugin: GUI - failed to recreate graphics context\n");
            return false;
        }
    }
    
    return true;
}

static bool gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    printf("MyPlugin: GUI - setting parent window (API: %s)\n", window ? window->api : "null");
    
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    gui_state_t *gui = &self->gui;
    
    if (!window) {
        return false;
    }
    
#ifdef _WIN32
    gui->parent_hwnd = (HWND)window->win32;
#elif defined(__APPLE__)
    gui->parent_nsview = window->cocoa;
#else
    gui->parent_window = window->x11;
    gui->display = XOpenDisplay(NULL);
#endif
    
    return create_platform_window(gui);
}

static bool gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window) {
    printf("MyPlugin: GUI - setting transient (floating window)\n");
    // For floating windows - not implemented in this basic example
    return true;
}

static void gui_suggest_title(const clap_plugin_t *plugin, const char *title) {
    printf("MyPlugin: GUI - suggested title: %s\n", title ? title : "null");
    // Store title for floating windows - not implemented in this basic example
}

static bool gui_show(const clap_plugin_t *plugin) {
    printf("MyPlugin: GUI - showing GUI\n");
    
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    gui_state_t *gui = &self->gui;
    
    if (!gui->is_created) {
        printf("MyPlugin: GUI - not created, cannot show\n");
        return false;
    }
    
    if (gui->is_visible) {
        printf("MyPlugin: GUI - already visible\n");
        return true;
    }
    
    // Render some graphics
    render_graphics(gui);
    
#ifdef _WIN32
    if (gui->hwnd) {
        ShowWindow(gui->hwnd, SW_SHOW);
        UpdateWindow(gui->hwnd);
    }
#elif defined(__APPLE__)
    // macOS show window code would go here
#else
    if (gui->display && gui->window) {
        XMapWindow(gui->display, gui->window);
        XFlush(gui->display);
    }
#endif
    
    gui->is_visible = true;
    printf("MyPlugin: GUI - shown successfully\n");
    return true;
}

static bool gui_hide(const clap_plugin_t *plugin) {
    printf("MyPlugin: GUI - hiding GUI\n");
    
    my_plugin_t *self = (my_plugin_t *)plugin->plugin_data;
    gui_state_t *gui = &self->gui;
    
    if (!gui->is_visible) {
        return true;
    }
    
#ifdef _WIN32
    if (gui->hwnd) {
        ShowWindow(gui->hwnd, SW_HIDE);
    }
#elif defined(__APPLE__)
    // macOS hide window code would go here
#else
    if (gui->display && gui->window) {
        XUnmapWindow(gui->display, gui->window);
        XFlush(gui->display);
    }
#endif
    
    gui->is_visible = false;
    printf("MyPlugin: GUI - hidden\n");
    return true;
}

// --- Platform-specific Window Management ---

static bool create_platform_window(gui_state_t *gui) {
    printf("MyPlugin: Creating platform window\n");
    
#ifdef _WIN32
    // Register window class
    static bool class_registered = false;
    if (!class_registered) {
        WNDCLASSEX wc = {0};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.lpfnWndProc = window_proc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = L"ClapJulesGUI";
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        
        if (!RegisterClassEx(&wc)) {
            printf("MyPlugin: Failed to register window class\n");
            return false;
        }
        class_registered = true;
    }
    
    // Create child window
    gui->hwnd = CreateWindowEx(
        0,
        L"ClapJulesGUI",
        L"CLAP Jules Plugin",
        WS_CHILD | WS_VISIBLE,
        0, 0, gui->width, gui->height,
        gui->parent_hwnd,
        NULL,
        GetModuleHandle(NULL),
        gui
    );
    
    if (!gui->hwnd) {
        printf("MyPlugin: Failed to create window\n");
        return false;
    }
    
#elif defined(__APPLE__)
    // macOS window creation would go here
    printf("MyPlugin: macOS window creation not implemented\n");
    return false;
    
#else
    // X11 window creation
    if (!gui->display) {
        printf("MyPlugin: No X11 display available\n");
        return false;
    }
    
    int screen = DefaultScreen(gui->display);
    
    gui->window = XCreateSimpleWindow(
        gui->display,
        gui->parent_window,
        0, 0, gui->width, gui->height,
        0,
        BlackPixel(gui->display, screen),
        WhitePixel(gui->display, screen)
    );
    
    if (!gui->window) {
        printf("MyPlugin: Failed to create X11 window\n");
        return false;
    }
    
    // Set window properties
    XStoreName(gui->display, gui->window, "CLAP Jules Plugin");
    
    // Select events we want to receive
    XSelectInput(gui->display, gui->window, ExposureMask | KeyPressMask | ButtonPressMask);
    
#endif
    
    printf("MyPlugin: Platform window created successfully\n");
    return true;
}

static void destroy_platform_window(gui_state_t *gui) {
    printf("MyPlugin: Destroying platform window\n");
    
#ifdef _WIN32
    if (gui->hwnd) {
        DestroyWindow(gui->hwnd);
        gui->hwnd = NULL;
    }
#elif defined(__APPLE__)
    // macOS window destruction would go here
#else
    if (gui->display) {
        if (gui->window) {
            XDestroyWindow(gui->display, gui->window);
            gui->window = 0;
        }
        XCloseDisplay(gui->display);
        gui->display = NULL;
    }
#endif
}

static void render_graphics(gui_state_t *gui) {
    if (!gui->graphics) {
        return;
    }
    
    // Clear background
    gui->graphics->clear(0xFF2D2D30);  // Dark background
    
    // Draw some test graphics
    gui->graphics->drawRect(10, 10, gui->width - 20, gui->height - 20, 0xFF0078D4);  // Blue border
    gui->graphics->drawRect(20, 20, gui->width - 40, gui->height - 40, 0xFF1E1E1E);  // Dark inner area
    
    // Draw title text area
    gui->graphics->drawRect(30, 30, gui->width - 60, 40, 0xFF404040);  // Title bar
    
    // Draw some sample elements
    int center_x = gui->width / 2;
    int center_y = gui->height / 2;
    
    gui->graphics->drawCircle(center_x, center_y, 50, 0xFFFF6B35);     // Orange circle
    gui->graphics->drawCircle(center_x, center_y, 30, 0xFFF7931E);     // Yellow inner circle
    gui->graphics->drawCircle(center_x, center_y, 10, 0xFFFFFFFF);     // White center dot
    
    // Draw corner decorations
    gui->graphics->drawLine(0, 0, 30, 30, 0xFF00FF00);               // Green diagonal
    gui->graphics->drawLine(gui->width - 1, 0, gui->width - 31, 30, 0xFF00FF00);  // Green diagonal
    gui->graphics->drawLine(0, gui->height - 1, 30, gui->height - 31, 0xFF00FF00); // Green diagonal
    gui->graphics->drawLine(gui->width - 1, gui->height - 1, gui->width - 31, gui->height - 31, 0xFF00FF00); // Green diagonal
    
    printf("MyPlugin: Graphics rendered\n");
}

#ifdef _WIN32
static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    gui_state_t *gui = NULL;
    
    if (msg == WM_CREATE) {
        CREATESTRUCT *cs = (CREATESTRUCT*)lParam;
        gui = (gui_state_t*)cs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)gui);
    } else {
        gui = (gui_state_t*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    
    switch (msg) {
        case WM_PAINT: {
            if (gui && gui->graphics) {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                
                // Create a bitmap from our graphics context
                const unsigned int* pixels = gui->graphics->getPixels();
                if (pixels) {
                    BITMAPINFO bmi = {0};
                    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                    bmi.bmiHeader.biWidth = gui->width;
                    bmi.bmiHeader.biHeight = -(int)gui->height; // Top-down DIB
                    bmi.bmiHeader.biPlanes = 1;
                    bmi.bmiHeader.biBitCount = 32;
                    bmi.bmiHeader.biCompression = BI_RGB;
                    
                    StretchDIBits(hdc, 0, 0, gui->width, gui->height,
                                 0, 0, gui->width, gui->height,
                                 pixels, &bmi, DIB_RGB_COLORS, SRCCOPY);
                }
                
                EndPaint(hwnd, &ps);
            }
            return 0;
        }
        
        case WM_SIZE: {
            if (gui) {
                gui->width = LOWORD(lParam);
                gui->height = HIWORD(lParam);
                printf("MyPlugin: Window resized to %dx%d\n", gui->width, gui->height);
                
                // Recreate graphics context with new size
                if (gui->graphics) {
                    delete gui->graphics;
                    try {
                        gui->graphics = new clap_jules::GraphicsContext(gui->width, gui->height);
                        render_graphics(gui);
                    } catch (...) {
                        printf("MyPlugin: Failed to recreate graphics context on resize\n");
                    }
                }
                
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;
        }
        
        case WM_DESTROY:
            return 0;
            
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}
#endif

#ifdef __linux__
static void handle_x11_events(gui_state_t *gui) {
    if (!gui->display) return;
    
    XEvent event;
    while (XPending(gui->display)) {
        XNextEvent(gui->display, &event);
        
        switch (event.type) {
            case Expose:
                if (event.xexpose.count == 0) {
                    // Redraw the window
                    const unsigned int* pixels = gui->graphics->getPixels();
                    if (pixels) {
                        // Convert pixels to X11 format and draw
                        // This is a simplified version - real implementation would need proper pixel format conversion
                        printf("MyPlugin: X11 expose event - would redraw here\n");
                    }
                }
                break;
                
            case ConfigureNotify:
                if (event.xconfigure.width != gui->width || event.xconfigure.height != gui->height) {
                    gui->width = event.xconfigure.width;
                    gui->height = event.xconfigure.height;
                    printf("MyPlugin: X11 window resized to %dx%d\n", gui->width, gui->height);
                    
                    // Recreate graphics context
                    if (gui->graphics) {
                        delete gui->graphics;
                        try {
                            gui->graphics = new clap_jules::GraphicsContext(gui->width, gui->height);
                            render_graphics(gui);
                        } catch (...) {
                            printf("MyPlugin: Failed to recreate graphics context on X11 resize\n");
                        }
                    }
                }
                break;
        }
    }
}
#endif

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
