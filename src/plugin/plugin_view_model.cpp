#include "plugin/plugin_view_model.hpp"

namespace plugin {

PluginViewModel::PluginViewModel(std::shared_ptr<audio::IAudioProcessor> processor)
    : audioProcessor_(processor)
    , gain(1.0f, this, "gain")
    , frequency(1000.0f, this, "frequency")
    , bypass(false, this, "bypass")
    , status("Ready", this, "status")
{
    resetCommand = std::make_shared<mvvm::RelayCommand>([this]() { onReset(); });
    toggleBypassCommand = std::make_shared<mvvm::RelayCommand>([this]() { onToggleBypass(); });
    
    addPropertyChangedListener([this](const std::string& propertyName) {
        onParameterChanged(propertyName);
    });
}

void PluginViewModel::updateStatus(const std::string& newStatus) {
    status.set(newStatus);
}

void PluginViewModel::processAudio(audio::AudioBuffer& buffer) {
    if (audioProcessor_ && !bypass.get()) {
        audioProcessor_->process(buffer);
    }
}

void PluginViewModel::setSampleRate(double sampleRate) {
    if (audioProcessor_) {
        audioProcessor_->setSampleRate(sampleRate);
    }
}

void PluginViewModel::setBufferSize(uint32_t bufferSize) {
    if (audioProcessor_) {
        audioProcessor_->setBufferSize(bufferSize);
    }
}

void PluginViewModel::onReset() {
    if (audioProcessor_) {
        audioProcessor_->reset();
    }
    updateStatus("Reset");
}

void PluginViewModel::onToggleBypass() {
    bypass.set(!bypass.get());
    updateStatus(bypass.get() ? "Bypassed" : "Active");
}

void PluginViewModel::onParameterChanged(const std::string& parameterName) {
    // Handle parameter changes here
    // This could trigger updates to the audio processor or UI
    if (parameterName == "gain" || parameterName == "frequency") {
        updateStatus("Parameter updated: " + parameterName);
    }
}

}