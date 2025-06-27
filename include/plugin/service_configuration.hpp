#pragma once

#include "di/container.hpp"
#include "audio/audio_processor.hpp"
#include "plugin/plugin_view_model.hpp"
#include "plugin/plugin_window.hpp"
#include <memory>

namespace plugin {

class ServiceConfiguration {
public:
    static void configure() {
        // Bind audio processor interface to implementation
        dependency_injection::Container::bind<audio::IAudioProcessor, audio::AudioProcessor>();
        
        // Simple configuration
        dependency_injection::Container::configure();
    }
    
    static std::shared_ptr<PluginViewModel> createViewModel() {
        auto processor = std::make_shared<audio::AudioProcessor>();
        return std::make_shared<PluginViewModel>(processor);
    }
    
    static std::shared_ptr<PluginWindow> createWindow(std::shared_ptr<PluginViewModel> viewModel) {
        // Create window manually since it needs viewModel parameter
        return std::make_shared<PluginWindow>(viewModel);
    }
    
    static std::shared_ptr<audio::IAudioProcessor> getAudioProcessor() {
        return std::make_shared<audio::AudioProcessor>();
    }
};

}