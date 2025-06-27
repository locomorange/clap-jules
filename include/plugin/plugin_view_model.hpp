#pragma once

#include "mvvm/view_model.hpp"
#include "mvvm/command.hpp"
#include "audio/audio_processor.hpp"
#include <memory>
#include <string>

namespace plugin {

class PluginViewModel : public mvvm::ViewModelBase {
public:
    PluginViewModel(std::shared_ptr<audio::IAudioProcessor> processor);
    virtual ~PluginViewModel() = default;
    
    // Properties
    mvvm::Property<float> gain;
    mvvm::Property<float> frequency;
    mvvm::Property<bool> bypass;
    mvvm::Property<std::string> status;
    
    // Commands
    std::shared_ptr<mvvm::RelayCommand> resetCommand;
    std::shared_ptr<mvvm::RelayCommand> toggleBypassCommand;
    
    // Methods
    void updateStatus(const std::string& newStatus);
    void processAudio(audio::AudioBuffer& buffer);
    void setSampleRate(double sampleRate);
    void setBufferSize(uint32_t bufferSize);
    
private:
    std::shared_ptr<audio::IAudioProcessor> audioProcessor_;
    
    void onReset();
    void onToggleBypass();
    void onParameterChanged(const std::string& parameterName);
};

}