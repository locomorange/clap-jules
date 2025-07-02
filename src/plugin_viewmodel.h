#pragma once

#include "plugin_model.h"
#include "audio_processor.h"
#include <boost/di.hpp>
#include <memory>

namespace plugin {

// View Model - handles UI logic and bridges View and Model
class PluginViewModel {
public:
    PluginViewModel(std::shared_ptr<PluginModel> model, 
                   std::shared_ptr<AudioProcessor> processor)
        : model_(model), processor_(processor) {}
    
    // UI parameter callbacks
    void OnCutoffFrequencyChanged(double frequency) {
        processor_->UpdateCutoffFrequency(frequency);
    }
    
    // Get current parameter values for UI display
    double GetCurrentCutoffFrequency() const {
        return model_->GetCutoffFrequency();
    }
    
    double GetCurrentSampleRate() const {
        return model_->GetSampleRate();
    }
    
private:
    std::shared_ptr<PluginModel> model_;
    std::shared_ptr<AudioProcessor> processor_;
};

// Dependency injection configuration
namespace di = boost::di;

auto CreateDIContainer() {
    return di::make_injector(
        di::bind<PluginModel>().in(di::singleton),
        di::bind<AudioProcessor>().in(di::singleton),
        di::bind<PluginViewModel>().in(di::singleton)
    );
}

} // namespace plugin