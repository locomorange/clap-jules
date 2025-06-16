#pragma once

#include <clap/clap.h>
#include <clap/ext/gui.h>

// Optional graphics library support
#ifdef HAVE_GLFW
#include <GLFW/glfw3.h>
#endif

#ifdef HAVE_SKIA
#include <skia/core/SkCanvas.h>
#include <skia/core/SkSurface.h>
#endif

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    // Add any other plugin-specific data here
#ifdef HAVE_GLFW
    GLFWwindow* window;
    bool gui_created;
    bool button_red;  // State for color-changing button
    int window_width;
    int window_height;
#endif
#ifdef HAVE_SKIA
    sk_sp<SkSurface> surface;
#endif
} my_plugin_t;

// GUI extension functions
#ifdef HAVE_GLFW
extern const clap_plugin_gui_t my_plugin_gui;
#endif

// Plugin factory ID
extern const struct clap_plugin_factory my_plugin_factory;
