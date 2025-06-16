#include "my_plugin.h"
#include <stdio.h>
#include <string.h>
#include <cstdlib>

// Plugin descriptor definition (moved from my_plugin.cpp)
static const char *const plugin_features[] = {"audio-effect", nullptr};

const clap_plugin_descriptor_t my_plugin_descriptor = {
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
};

// Plugin factory functions (moved from my_plugin.cpp)
static uint32_t my_factory_get_plugin_count(const struct clap_plugin_factory *factory) {
    return 1; // We have one plugin in this factory
}

static const clap_plugin_descriptor_t *my_factory_get_plugin_descriptor(const struct clap_plugin_factory *factory, uint32_t index) {
    if (index == 0) {
        return &my_plugin_descriptor;
    }
    return NULL;
}

// Forward declaration of plugin creation function
extern const clap_plugin_t *create_my_plugin_instance(const clap_host_t *host, const char *plugin_id);

static const clap_plugin_t *my_factory_create_plugin(const struct clap_plugin_factory *factory, const clap_host_t *host, const char *plugin_id) {
    // Validate inputs
    if (!factory || !host || !plugin_id) {
        fprintf(stderr, "MyPlugin: Error - null parameters passed to create_plugin\n");
        return NULL;
    }
    
    printf("MyPlugin: Creating plugin with ID: %s (expected: %s)\n", plugin_id, my_plugin_descriptor.id);
    
    if (strcmp(plugin_id, my_plugin_descriptor.id) != 0) {
        fprintf(stderr, "MyPlugin: Error - incorrect plugin ID requested: %s (expected: %s)\n", 
                plugin_id, my_plugin_descriptor.id);
        return NULL;
    }

    return create_my_plugin_instance(host, plugin_id);
}

// Plugin factory definition (moved from my_plugin.cpp)
CLAP_EXPORT const struct clap_plugin_factory my_plugin_factory = {
    my_factory_get_plugin_count,
    my_factory_get_plugin_descriptor,
    my_factory_create_plugin,
};