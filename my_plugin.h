#pragma once

#include <clap/clap.h>
#include <memory>

// Forward declarations
namespace ClapeJules {
    class AudioProcessor;
    class FilterViewModel;
    class FilterView;
}

// Plugin structure with MVVM components
typedef struct {
    clap_plugin_t plugin;
    
    // MVVM components managed by dependency injection
    std::shared_ptr<ClapeJules::AudioProcessor> audioProcessor;
    std::shared_ptr<ClapeJules::FilterViewModel> viewModel;
    std::shared_ptr<ClapeJules::FilterView> view;
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;
