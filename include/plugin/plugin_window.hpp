#pragma once

#include <brisk/core/BasicTypes.hpp>
#include <brisk/gui/GUI.hpp>
#include <brisk/gui/Window.hpp>
#include <brisk/widgets/Widgets.hpp>
#include "plugin/plugin_view_model.hpp"
#include <memory>

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
    std::shared_ptr<brisk::Window> window_;
    
    void createUI();
    void bindViewModel();
    void setupEventHandlers();
    
    // UI Elements
    std::shared_ptr<brisk::Slider> gainSlider_;
    std::shared_ptr<brisk::Slider> frequencySlider_;
    std::shared_ptr<brisk::Button> bypassButton_;
    std::shared_ptr<brisk::Button> resetButton_;
    std::shared_ptr<brisk::Label> statusLabel_;
};

}