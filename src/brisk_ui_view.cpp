#include "brisk_ui_view.h"
#include <brisk/brisk.h>
#include <exception>
#include <cstdio>

namespace plugin {

// Static initialization tracking for Brisk
bool BriskUIView::brisk_initialized_ = false;

// Initialize the UI window
bool BriskUIView::Initialize(void* parent_handle) {
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
void BriskUIView::SetVisible(bool visible) {
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
void* BriskUIView::GetWindowHandle() const {
    return window_ ? window_->GetHandle() : parent_handle_;
}

// Update UI with current parameter values
void BriskUIView::UpdateUI() {
    current_cutoff_ = viewmodel_->GetCurrentCutoffFrequency();
    if (frequency_knob_) {
        frequency_knob_->SetValue(current_cutoff_);
    }
}

// Handle parameter changes from UI
void BriskUIView::OnFrequencyKnobChanged(double value) {
    viewmodel_->OnCutoffFrequencyChanged(value);
}

// Sample drawing function using brisk
void BriskUIView::DrawSampleContent() {
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
void BriskUIView::DrawFrequencyResponse() {
    // TODO: Implement frequency response visualization
    // This would draw a graph showing the filter's frequency response
    printf("BriskUIView: Drawing frequency response graph\n");
}

// Draw filter characteristics display
void BriskUIView::DrawFilterCharacteristics() {
    // TODO: Display current filter parameters
    // Show cutoff frequency, filter type, etc.
    printf("BriskUIView: Drawing filter characteristics (Cutoff: %.1f Hz)\n", current_cutoff_);
}

// Handle window resize
void BriskUIView::SetSize(uint32_t width, uint32_t height) {
    printf("BriskUIView: Resizing to %ux%u\n", width, height);
    if (window_) {
        // TODO: Implement window resizing in Brisk
        // window_->SetSize(width, height);
    }
}

// Cleanup
BriskUIView::~BriskUIView() {
    frequency_knob_.reset();
    window_.reset();
    // Note: Don't shutdown brisk application here as it might be shared
}

} // namespace plugin