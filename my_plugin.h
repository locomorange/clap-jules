#pragma once

#include <clap/clap.h>
#include <clap/ext/gui.h>

// Forward declaration for GUI data
typedef struct gui_data gui_data_t;

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    
    // GUI-related data
    gui_data_t *gui_data;
} my_plugin_t;

// Plugin factory ID
// On Windows, we need different declarations for DLL export vs import
#ifdef _WIN32
    #ifdef BUILDING_MY_CLAP_PLUGIN
        extern CLAP_EXPORT const struct clap_plugin_factory my_plugin_factory;
    #else
        extern __declspec(dllimport) const struct clap_plugin_factory my_plugin_factory;
    #endif
#else
    extern CLAP_EXPORT const struct clap_plugin_factory my_plugin_factory;
#endif

// Plugin descriptor (defined in plugin_factory.cpp)
#ifdef _WIN32
    #ifdef BUILDING_MY_CLAP_PLUGIN
        extern const clap_plugin_descriptor_t my_plugin_descriptor;
    #else
        extern __declspec(dllimport) const clap_plugin_descriptor_t my_plugin_descriptor;
    #endif
#else
    extern const clap_plugin_descriptor_t my_plugin_descriptor;
#endif

// Plugin instance creation function
#ifdef _WIN32
    #ifdef BUILDING_MY_CLAP_PLUGIN
        extern CLAP_EXPORT const clap_plugin_t *create_my_plugin_instance(const clap_host_t *host, const char *plugin_id);
    #else
        extern __declspec(dllimport) const clap_plugin_t *create_my_plugin_instance(const clap_host_t *host, const char *plugin_id);
    #endif
#else
    extern const clap_plugin_t *create_my_plugin_instance(const clap_host_t *host, const char *plugin_id);
#endif

// CLAP entry point
#ifdef _WIN32
    #ifdef BUILDING_MY_CLAP_PLUGIN
        extern CLAP_EXPORT const clap_plugin_entry_t clap_entry;
    #else
        extern __declspec(dllimport) const clap_plugin_entry_t clap_entry;
    #endif
#else
    extern CLAP_EXPORT const clap_plugin_entry_t clap_entry;
#endif
