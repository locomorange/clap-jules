#pragma once

#include <clap/clap.h>
#include <memory>

// Forward declarations for C++ classes
namespace plugin {
    class PluginViewModel;
    class PluginWindow;
}

// Basic plugin structure
typedef struct {
    clap_plugin_t plugin;
    
    // MVVM components
    std::shared_ptr<plugin::PluginViewModel> viewModel;
    std::shared_ptr<plugin::PluginWindow> window;
    
    // Plugin state
    double sampleRate;
    uint32_t bufferSize;
    bool isProcessing;
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
