#pragma once

#include <clap/clap.h>

#ifdef __cplusplus
extern "C" {
#endif

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    // Graphics renderer pointer (void* to avoid C++ linkage issues)
    void* graphics_renderer;
    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;

#ifdef __cplusplus
}

// C++ interface for graphics functionality
namespace clap_jules {
    class GraphicsRenderer;
    void initializeGraphics(my_plugin_t* plugin);
    void cleanupGraphics(my_plugin_t* plugin);
    void renderFrame(my_plugin_t* plugin, int width, int height);
}
#endif
