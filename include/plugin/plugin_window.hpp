#pragma once

#include "plugin/plugin_view_model.hpp"
#include <memory>

// Try to include Brisk headers, fallback to minimal implementation if not available
#if __has_include(<brisk/gui/GuiWindow.hpp>)
#include <brisk/gui/GuiWindow.hpp>
#include <brisk/widgets/Button.hpp>
#include <brisk/widgets/Slider.hpp>
#include <brisk/widgets/Text.hpp>
#include <brisk/widgets/Layouts.hpp>
#define BRISK_AVAILABLE 1
#else
#define BRISK_AVAILABLE 0
#endif

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