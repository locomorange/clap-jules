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
        
        // Position the knob in center of window
        frequency_knob_->SetPosition(200, 150); // Center of 400x300 window
        frequency_knob_->SetRadius(40); // Larger radius for better visibility
        
        // Set up knob callback
        frequency_knob_->SetCallback([this](double value) {
            this->OnFrequencyKnobChanged(value);
        });
        
        parent_handle_ = parent_handle;
        
        // Set initial window size
        if (window_) {
            window_->SetSize(400, 300);
        }
        
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
    
    // Trigger redraw
    DrawSampleContent();
}

// Handle parameter changes from UI
void BriskUIView::OnFrequencyKnobChanged(double value) {
    current_cutoff_ = value;
    viewmodel_->OnCutoffFrequencyChanged(value);
    printf("BriskUIView: Frequency knob changed to %.1f Hz\n", value);
}

// Sample drawing function using brisk
void BriskUIView::DrawSampleContent() {
    printf("BriskUIView: Drawing sample content\n");
    if (window_) {
        try {
            // Begin drawing
            window_->BeginDraw();
            
            // Draw UI title
            window_->DrawText(10, 25, "CLAP Low-Pass Filter Plugin");
            
            // Draw frequency label
            char freq_label[64];
            snprintf(freq_label, sizeof(freq_label), "Cutoff: %.1f Hz", current_cutoff_);
            window_->DrawText(150, 80, freq_label);
            
            // Draw the frequency knob
            if (frequency_knob_) {
                frequency_knob_->Draw(window_.get());
            }
            
            // Draw frequency response visualization
            DrawFrequencyResponse();
            
            // Draw filter characteristics
            DrawFilterCharacteristics();
            
            // End drawing
            window_->EndDraw();
            
            printf("BriskUIView: Drawing completed successfully\n");
        } catch (const std::exception& e) {
            printf("BriskUIView: Exception during drawing: %s\n", e.what());
        }
    }
}

// Draw frequency response graph
void BriskUIView::DrawFrequencyResponse() {
    if (!window_) return;
    
    // Draw a simple frequency response visualization
    window_->DrawText(10, 220, "Frequency Response:");
    window_->DrawText(10, 240, "20Hz              1kHz              20kHz");
    window_->DrawText(10, 260, "[-----|-----|-----|-----|-----|-----]");
    
    printf("BriskUIView: Drawing frequency response graph\n");
}

// Draw filter characteristics display
void BriskUIView::DrawFilterCharacteristics() {
    if (!window_) return;
    
    // Display current filter parameters
    char info[128];
    snprintf(info, sizeof(info), "Low-Pass Filter @ %.1f Hz", current_cutoff_);
    window_->DrawText(10, 50, info);
    
    printf("BriskUIView: Drawing filter characteristics (Cutoff: %.1f Hz)\n", current_cutoff_);
}

// Handle window resize
void BriskUIView::SetSize(uint32_t width, uint32_t height) {
    printf("BriskUIView: Resizing to %ux%u\n", width, height);
    if (window_) {
        window_->SetSize(width, height);
        
        // Reposition knob to center of new size
        if (frequency_knob_) {
            frequency_knob_->SetPosition(width / 2, height / 2);
        }
        
        // Redraw with new layout
        DrawSampleContent();
    }
}

// Cleanup
BriskUIView::~BriskUIView() {
    frequency_knob_.reset();
    window_.reset();
    // Note: Don't shutdown brisk application here as it might be shared
}

} // namespace plugin