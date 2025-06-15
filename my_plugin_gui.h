#pragma once

#include <clap/clap.h>
#include <clap/ext/gui.h>
#include <clap/ext/params.h>
#include <vstgui/vstgui.h>
#include <vstgui/lib/cframe.h>
#include <vstgui/lib/controls/cknob.h>
#include <vstgui/lib/controls/cslider.h>
#include <vstgui/lib/controls/ctextlabel.h>
#include <vstgui/lib/controls/cbuttons.h>
#include <vstgui/lib/controls/coptionmenu.h>
#include <vstgui/lib/controls/csegmentbutton.h>

using namespace VSTGUI;

// Forward declaration
struct my_plugin_t;

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
    
    // Parameter binding
    void setPlugin(struct my_plugin_t* plugin) { this->plugin = plugin; }
    void updateParameter(uint32_t param_id, double value);
    
    // IControlListener implementation
    void valueChanged(CControl* control) override;
    
private:
    CFrame* frame;
    bool isCreated;
    bool isVisible;
    uint32_t currentWidth;
    uint32_t currentHeight;
    const clap_host_t* host;
    struct my_plugin_t* plugin;
    
    // Control pointers for parameter binding
    CKnob* depthKnob;
    CKnob* sharpnessKnob;
    CKnob* selectivityKnob;
    CKnob* balanceKnob;
    CKnob* frequencyKnob;
    CKnob* gainKnob;
    CKnob* qKnob;
    CSegmentButton* modeButton;
    COnOffButton* linkButton;
    COnOffButton* bypassButton;
    
    void createSoothe2StyleControls();
    void drawSpectrum(CDrawContext* context, const CRect& rect);
    void drawFilterCurve(CDrawContext* context, const CRect& rect);
};