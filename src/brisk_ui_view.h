#pragma once

#include "plugin_viewmodel.h"
#include <brisk/brisk.h>
#include <memory>
#include <functional>

namespace plugin {

// Brisk UI integration with MVVM pattern
class BriskUIView {
public:
    explicit BriskUIView(std::shared_ptr<PluginViewModel> viewmodel)
        : viewmodel_(viewmodel), window_(nullptr), frequency_knob_(nullptr) {}
    
    // Initialize the UI window
    bool Initialize(void* parent_handle) {
        try {
            // Initialize brisk application if not already done
            brisk::Application::Initialize();
            
            // Create brisk window
            window_ = std::make_unique<brisk::Window>(parent_handle);
            
            // Create frequency knob (20Hz to 20kHz range)
            frequency_knob_ = std::make_unique<brisk::Knob>(
                20.0, 20000.0, viewmodel_->GetCurrentCutoffFrequency()
            );
            
            // Set up knob callback
            frequency_knob_->SetCallback([this](double value) {
                this->OnFrequencyKnobChanged(value);
            });
            
            parent_handle_ = parent_handle;
            return true;
        } catch (const std::exception& e) {
            printf("BriskUIView: Failed to initialize: %s\n", e.what());
            return false;
        }
    }
    
    // Show/hide the window
    void SetVisible(bool visible) {
        visible_ = visible;
        if (window_) {
            window_->SetVisible(visible);
        }
    }
    
    // Get window handle for host
    void* GetWindowHandle() const {
        return window_ ? window_->GetHandle() : parent_handle_;
    }
    
    // Update UI with current parameter values
    void UpdateUI() {
        current_cutoff_ = viewmodel_->GetCurrentCutoffFrequency();
        if (frequency_knob_) {
            frequency_knob_->SetValue(current_cutoff_);
        }
    }
    
    // Handle parameter changes from UI
    void OnFrequencyKnobChanged(double value) {
        viewmodel_->OnCutoffFrequencyChanged(value);
    }
    
    // Sample drawing function using brisk
    void DrawSampleContent() {
        if (window_) {
            window_->Render();
        }
        // TODO: Add frequency response visualization
        // TODO: Add waveform display
        // TODO: Add filter characteristics graph
    }
    
    // Cleanup
    ~BriskUIView() {
        frequency_knob_.reset();
        window_.reset();
        // Note: Don't shutdown brisk application here as it might be shared
    }
    
private:
    std::shared_ptr<PluginViewModel> viewmodel_;
    std::unique_ptr<brisk::Window> window_;
    std::unique_ptr<brisk::Knob> frequency_knob_;
    void* parent_handle_ = nullptr;
    bool visible_ = false;
    double current_cutoff_ = 1000.0;
};

} // namespace plugin