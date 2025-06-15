#pragma once

#include <clap/clap.h>
#include <clap/ext/gui.h>
#include <clap/ext/params.h>
#include <vstgui/vstgui.h>
#include <vstgui/lib/cframe.h>
#include <vstgui/lib/controls/ccontrol.h>
#include "my_plugin.h"

using namespace VSTGUI;

class MyPluginEditor : public IControlListener {
public:
    MyPluginEditor(const clap_host_t* host = nullptr);
    ~MyPluginEditor();
    
    // CLAP GUI interface implementation
    bool isApiSupported(const char* api, bool isFloating);
    bool getPreferredApi(const char** api, bool* isFloating);
    bool create(const char* api, bool isFloating);
    void destroy();
    bool setScale(double scale);
    bool getSize(uint32_t* width, uint32_t* height);
    bool canResize();
    bool getResizeHints(clap_gui_resize_hints_t* hints);
    bool adjustSize(uint32_t* width, uint32_t* height);
    bool setSize(uint32_t width, uint32_t height);
    bool setParent(const clap_window_t* window);
    bool setTransient(const clap_window_t* window);
    void suggestTitle(const char* title);
    bool show();
    bool hide();
    
    // Parameter synchronization
    void updateParameter(clap_id paramId, double value);
    
    // IControlListener
    void valueChanged(CControl* pControl) override;
    
private:
    CFrame* frame;
    bool isCreated;
    bool isVisible;
    uint32_t currentWidth;
    uint32_t currentHeight;
    const clap_host_t* host;
    
    void createControls();
    void notifyParameterChange(clap_id paramId, double value);
};