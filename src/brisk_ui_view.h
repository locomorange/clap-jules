#pragma once

#include "plugin_viewmodel.h"
#include <memory>
#include <functional>

// Brisk integration headers
#include <brisk/core/Rc.hpp>
#include <brisk/core/App.hpp>
#include <brisk/window/Window.hpp>
#include <brisk/window/WindowApplication.hpp>
#include <brisk/widgets/Widgets.hpp>
#include <brisk/widgets/Knob.hpp>

namespace plugin {

// Brisk UI integration with MVVM pattern
class BriskUIView {
public:
    explicit BriskUIView(std::shared_ptr<PluginViewModel> viewmodel)
        : viewmodel_(viewmodel), 
          parent_handle_(nullptr),
          visible_(false),
          current_cutoff_(1000.0) {}
    
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
    Brisk::Rc<Brisk::Window> window_;
    Brisk::Rc<Brisk::Knob> frequency_knob_;
    void* parent_handle_;
    bool visible_;
    double current_cutoff_;
    static bool brisk_initialized_; // Static flag to track brisk initialization
};

} // namespace plugin