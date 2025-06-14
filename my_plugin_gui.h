#pragma once

#include <clap/clap.h>
#include <clap/ext/gui.h>
#include <vstgui/vstgui.h>
#include <vstgui/lib/cframe.h>

using namespace VSTGUI;

class MyPluginEditor {
public:
    MyPluginEditor();
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
    
private:
    CFrame* frame;
    bool isCreated;
    bool isVisible;
    uint32_t currentWidth;
    uint32_t currentHeight;
    
    void createControls();
};