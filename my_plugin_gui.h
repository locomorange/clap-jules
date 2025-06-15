#pragma once

#include <clap/clap.h>
#include <clap/ext/gui.h>
#include <vstgui/vstgui.h>
#include <vstgui/lib/cframe.h>
#include <vstgui/lib/controls/cknob.h>
#include <vstgui/lib/controls/coptionmenu.h>
#include <vstgui/lib/controls/cbuttons.h>
#include <vstgui/lib/controls/ctextlabel.h>
#include <vstgui/lib/cview.h>
#include "my_plugin.h"

using namespace VSTGUI;

// External reference to parameter info
extern const clap_param_info_t param_info[];

// Custom EQ Curve View
class EQCurveView : public CView {
public:
    EQCurveView(const CRect& size);
    ~EQCurveView();
    
    void draw(CDrawContext* pContext) override;
    CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;
    CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons) override;
    CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;
    
    void setPlugin(my_plugin_t* plugin) { this->plugin = plugin; }
    void updateCurve();
    
private:
    my_plugin_t* plugin;
    int32_t draggedBand;
    bool isDragging;
    
    void drawGrid(CDrawContext* pContext);
    void drawCurve(CDrawContext* pContext);
    void drawControlPoints(CDrawContext* pContext);
    CPoint freqGainToPoint(double freq, double gain);
    void pointToFreqGain(CPoint point, double& freq, double& gain);
};

class MyPluginEditor {
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
    
    void setPlugin(my_plugin_t* plugin) { this->plugin = plugin; }
    void updateParameters();
    
private:
    CFrame* frame;
    bool isCreated;
    bool isVisible;
    uint32_t currentWidth;
    uint32_t currentHeight;
    const clap_host_t* host;
    my_plugin_t* plugin;
    
    // GUI Controls
    CKnob* lowFreqKnob;
    CKnob* lowGainKnob;
    CKnob* lowQKnob;
    COptionMenu* lowTypeMenu;
    COnOffButton* lowBypassButton;
    
    CKnob* lowMidFreqKnob;
    CKnob* lowMidGainKnob;
    CKnob* lowMidQKnob;
    COptionMenu* lowMidTypeMenu;
    COnOffButton* lowMidBypassButton;
    
    CKnob* highMidFreqKnob;
    CKnob* highMidGainKnob;
    CKnob* highMidQKnob;
    COptionMenu* highMidTypeMenu;
    COnOffButton* highMidBypassButton;
    
    CKnob* highFreqKnob;
    CKnob* highGainKnob;
    CKnob* highQKnob;
    COptionMenu* highTypeMenu;
    COnOffButton* highBypassButton;
    
    CKnob* masterGainKnob;
    COnOffButton* masterBypassButton;
    
    // EQ Curve Display
    EQCurveView* eqCurveView;
    
    // UI sections
    CTextButton* undoButton;
    CTextButton* redoButton;
    COptionMenu* presetMenu;
    
    void createControls();
    void createControlPanel();
    void createEQCurvePanel();
    void createTopPanel();
    void styleControl(CView* control);
};