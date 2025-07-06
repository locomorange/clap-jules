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
            printf("BriskUIView: Initializing with parent handle: %p\n", parent_handle);
            
            // Initialize brisk application if not already done
            if (!brisk_initialized_) {
                brisk::Application::Initialize();
                brisk_initialized_ = true;
                printf("BriskUIView: Brisk application initialized\n");
            }
            
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
            printf("BriskUIView: UI components created successfully\n");
            return true;
        } catch (const std::exception& e) {
            printf("BriskUIView: Failed to initialize: %s\n", e.what());
            return false;
        }
    }
    
    // Show/hide the window
    void SetVisible(bool visible) {
        visible_ = visible;
        printf("BriskUIView: Setting visibility to %s\n", visible ? "true" : "false");
        if (window_) {
            window_->SetVisible(visible);
            if (visible) {
                // Trigger a redraw when shown
                DrawSampleContent();
            }
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
        printf("BriskUIView: Drawing sample content\n");
        if (window_) {
            try {
                window_->Render();
                
                // Draw frequency response visualization
                DrawFrequencyResponse();
                
                // Draw filter characteristics
                DrawFilterCharacteristics();
                
                printf("BriskUIView: Drawing completed successfully\n");
            } catch (const std::exception& e) {
                printf("BriskUIView: Exception during drawing: %s\n", e.what());
            }
        }
    }
    
    // Draw frequency response graph
    void DrawFrequencyResponse() {
        // TODO: Implement frequency response visualization
        // This would draw a graph showing the filter's frequency response
        printf("BriskUIView: Drawing frequency response graph\n");
    }
    
    // Draw filter characteristics display
    void DrawFilterCharacteristics() {
        // TODO: Display current filter parameters
        // Show cutoff frequency, filter type, etc.
        printf("BriskUIView: Drawing filter characteristics (Cutoff: %.1f Hz)\n", current_cutoff_);
    }
    
    // Handle window resize
    void SetSize(uint32_t width, uint32_t height) {
        printf("BriskUIView: Resizing to %ux%u\n", width, height);
        if (window_) {
            // TODO: Implement window resizing in Brisk
            // window_->SetSize(width, height);
        }
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
    static bool brisk_initialized_; // Static flag to track brisk initialization
};

} // namespace plugin