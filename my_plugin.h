#pragma once

#include <clap/clap.h>

#pragma once

#include <clap/clap.h>

// Optional graphics library support
#ifdef HAVE_GLFW
#define GLFW_INCLUDE_NONE  // Don't include OpenGL headers automatically
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
#endif
#ifdef HAVE_SKIA
    sk_sp<SkSurface> surface;
#endif
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
