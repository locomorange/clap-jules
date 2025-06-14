#pragma once

#include <clap/clap.h>
#include "vstgui/vstgui.h"

using namespace VSTGUI;

class PluginGUI {
public:
    PluginGUI(const clap_plugin_t* plugin);
    ~PluginGUI();
    
    bool createWindow(clap_window_t* window);
    void destroyWindow();
    bool setParent(const clap_window_t* window);
    bool setSize(uint32_t width, uint32_t height);
    bool getSize(uint32_t* width, uint32_t* height);
    bool canResize();
    bool getResizeHints(clap_gui_resize_hints_t* hints);
    bool adjustSize(uint32_t* width, uint32_t* height);
    void setScale(double scale);
    bool show();
    bool hide();
    
    static constexpr uint32_t DEFAULT_WIDTH = 400;
    static constexpr uint32_t DEFAULT_HEIGHT = 300;

private:
    const clap_plugin_t* plugin_;
    SharedPointer<CFrame> frame_;
    SharedPointer<CSlider> volumeSlider_;
    SharedPointer<CTextEdit> paramDisplay_;
    
    void setupGUI();
};

// CLAP GUI extension implementation
extern const clap_plugin_gui_t plugin_gui_extension;