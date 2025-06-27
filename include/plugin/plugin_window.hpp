#pragma once

#include "plugin/plugin_view_model.hpp"
#include <memory>

// Minimal implementation without external GUI dependencies
#define BRISK_AVAILABLE 0

namespace plugin {

class PluginWindow {
public:
    PluginWindow(std::shared_ptr<PluginViewModel> viewModel);
    virtual ~PluginWindow() = default;
    
    void show();
    void hide();
    bool isVisible() const;
    
    void setSize(int width, int height);
    void setPosition(int x, int y);
    
    // Native window handle for embedding in DAW
    void* getNativeHandle() const;
    
private:
    std::shared_ptr<PluginViewModel> viewModel_;
    
#if BRISK_AVAILABLE
    std::shared_ptr<brisk::GuiWindow> window_;
    std::shared_ptr<brisk::Slider> gainSlider_;
    std::shared_ptr<brisk::Slider> frequencySlider_;
    std::shared_ptr<brisk::Button> bypassButton_;
    std::shared_ptr<brisk::Button> resetButton_;
    std::shared_ptr<brisk::Text> statusLabel_;
#else
    // Minimal implementation when Brisk is not available
    void* window_;
    bool visible_;
#endif
    
    void createUI();
    void bindViewModel();
    void setupEventHandlers();
};

}