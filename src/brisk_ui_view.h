#pragma once

#include "plugin_viewmodel.h"
#include <memory>
#include <functional>

#include "brisk/brisk.h"

namespace plugin {

// Brisk UI integration with MVVM pattern
class BriskUIView {
public:
    explicit BriskUIView(std::shared_ptr<PluginViewModel> viewmodel)
        : viewmodel_(viewmodel), window_(nullptr), frequency_knob_(nullptr) {}
    
    // Initialize the UI window
    bool Initialize(void* parent_handle);
    
    // Show/hide the window
    void SetVisible(bool visible);
    
    // Get window handle for host
    void* GetWindowHandle() const;
    
    // Update UI with current parameter values
    void UpdateUI();
    
    // Handle parameter changes from UI
    void OnFrequencyKnobChanged(double value);
    
    // Sample drawing function using brisk
    void DrawSampleContent();
    
    // Draw frequency response graph
    void DrawFrequencyResponse();
    
    // Draw filter characteristics display
    void DrawFilterCharacteristics();
    
    // Handle window resize
    void SetSize(uint32_t width, uint32_t height);
    
    // Cleanup
    ~BriskUIView();
    
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