#pragma once

#include <clap/clap.h>
#include <memory>

// Forward declarations
class PluginViewModel;

// Enhanced plugin structure with MVVM support
typedef struct {
    clap_plugin_t plugin;
    std::shared_ptr<PluginViewModel> viewModel;
    double currentFrequency;
    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
