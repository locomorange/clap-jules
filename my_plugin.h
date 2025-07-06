#pragma once

#include <clap/clap.h>
#include <clap/ext/gui.h>
#include "src/plugin_viewmodel.h"
#include "src/audio_processor.h"
#include "src/brisk_ui_view.h"
#include <memory>

// Plugin structure with MVVM components
typedef struct {
    clap_plugin_t plugin;
    
    // MVVM components
    std::shared_ptr<plugin::PluginModel> model;
    std::shared_ptr<plugin::AudioProcessor> processor;
    std::shared_ptr<plugin::PluginViewModel> viewmodel;
    std::shared_ptr<plugin::BriskUIView> ui_view;
    
    // Plugin state
    double sample_rate;
    uint32_t min_frames;
    uint32_t max_frames;
    bool is_processing;
    
    // GUI state
    bool gui_created;
    bool gui_visible;
    uint32_t gui_width;
    uint32_t gui_height;
    void* parent_window;
    const clap_host_t* host;
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
