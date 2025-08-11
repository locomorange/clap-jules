#include "brisk_ui_view.h"
#include <brisk/core/Rc.hpp>
#include <brisk/core/App.hpp>
#include <brisk/window/Window.hpp>
#include <brisk/window/WindowApplication.hpp>
#include <brisk/widgets/Widgets.hpp>
#include <brisk/widgets/Knob.hpp>
#include <exception>
#include <cstdio>

namespace plugin {

// Static initialization tracking for Brisk
bool BriskUIView::brisk_initialized_ = false;

// Initialize the UI window
bool BriskUIView::Initialize(void* parent_handle) {
    try {
        printf("BriskUIView: Initializing with parent handle: %p\n", parent_handle);
        
        parent_handle_ = parent_handle;
        
        // Create a Brisk window
        window_ = rcnew Brisk::Window();
        if (!window_) {
            printf("BriskUIView: Failed to create window\n");
            return false;
        }
        
        // Set window properties
        window_->setSize({400, 300});
        window_->setTitle("CLAP Plugin UI");
        
        printf("BriskUIView: UI initialized successfully with Brisk\n");
        return true;
    } catch (const std::exception& e) {
        printf("BriskUIView: Failed to initialize: %s\n", e.what());
        return false;
    }
}

// Show/hide the window
void BriskUIView::SetVisible(bool visible) {
    visible_ = visible;
    printf("BriskUIView: SetVisible called with %s\n", visible ? "true" : "false");
    
    if (window_) {
        if (visible) {
            window_->show();
        } else {
            window_->hide();
        }
    }
}

// Get window handle for host
void* BriskUIView::GetWindowHandle() const {
    // TODO: Fix NativeWindowHandle to void* conversion issue
    // The actual implementation would be:
    // if (window_) {
    //     return reinterpret_cast<void*>(window_->getHandle());
    // }
    // For now, return the parent handle as a placeholder
    return parent_handle_;
}

// Update UI with current parameter values
void BriskUIView::UpdateUI() {
    current_cutoff_ = viewmodel_->GetCurrentCutoffFrequency();
    printf("BriskUIView: UpdateUI called, cutoff frequency: %f Hz\n", current_cutoff_);
}

// Handle parameter changes from UI
void BriskUIView::OnFrequencyKnobChanged(double value) {
    printf("BriskUIView: Frequency knob changed to %f Hz\n", value);
    current_cutoff_ = value;
    viewmodel_->OnCutoffFrequencyChanged(value);
}

// Sample drawing function using brisk (placeholder)
void BriskUIView::DrawSampleContent() {
    printf("BriskUIView: DrawSampleContent called\n");
}

// Draw frequency response graph (placeholder)  
void BriskUIView::DrawFrequencyResponse() {
    printf("BriskUIView: DrawFrequencyResponse called\n");
}

// Draw filter characteristics display (placeholder)
void BriskUIView::DrawFilterCharacteristics() {
    printf("BriskUIView: DrawFilterCharacteristics called\n");
}

// Handle window resize
void BriskUIView::SetSize(uint32_t width, uint32_t height) {
    printf("BriskUIView: SetSize called with %dx%d\n", width, height);
    
    if (window_) {
        window_->setSize({static_cast<int>(width), static_cast<int>(height)});
    }
}

// Cleanup
BriskUIView::~BriskUIView() {
    printf("BriskUIView: Destructor called\n");
}

} // namespace plugin
