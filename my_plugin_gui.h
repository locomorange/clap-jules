#pragma once

#include <clap/clap.h>
#include <clap/ext/gui.h>
#include <vstgui/vstgui.h>
#include <vstgui/lib/cframe.h>
#include <vstgui/lib/cview.h>
#include <vstgui/lib/cviewcontainer.h>
#include <vstgui/lib/controls/cknob.h>
#include <vstgui/lib/controls/cslider.h>
#include <vstgui/lib/controls/ctextlabel.h>
#include <vstgui/lib/controls/coptionmenu.h>
#include <vstgui/lib/controls/cbuttons.h>
#include <vstgui/lib/cdrawcontext.h>
#include <vstgui/lib/cgraphicspath.h>
#include <memory>
#include <vector>
#include <mutex>
#include "my_plugin.h"

using namespace VSTGUI;

// EQ Node structure for interactive EQ display
struct EQNode {
    double freq;
    double gain;
    double q;
    CPoint position;
    bool selected;
    bool dragging;
};

// Forward declarations (to avoid circular dependencies)
class MyPluginEditor;
class SpectrumAnalyzerView;

// Custom EQ visualization view
class EQVisualizationView : public CView {
public:
    EQVisualizationView(const CRect& size);
    ~EQVisualizationView();
    
    void draw(CDrawContext* context) override;
    CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;
    CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons) override;
    CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;
    
    void updateEQData(double eq_freq[3], double eq_gain[3], double eq_q[3]);
    void setNodeSelected(int nodeIndex, bool selected);
    
private:
    std::vector<EQNode> eqNodes;
    int selectedNode;
    CPoint lastMousePos;
    
    void drawGrid(CDrawContext* context);
    void drawFrequencyResponse(CDrawContext* context);
    void drawNodes(CDrawContext* context);
    CPoint frequencyToPosition(double freq, double gain);
    void positionToFrequency(const CPoint& pos, double& freq, double& gain);
    int getNodeAtPosition(const CPoint& pos);
};

// Custom spectrum analyzer visualization view
class SpectrumAnalyzerView : public CView {
public:
    SpectrumAnalyzerView(const CRect& size);
    ~SpectrumAnalyzerView();
    
    void draw(CDrawContext* context) override;
    CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;
    
    void updateSpectrumData(const std::vector<float>& magnitudes, const std::vector<float>& frequencies);
    void setDrawStyle(SpectrumDrawStyle style);
    void setEnabled(bool enabled);
    
private:
    std::vector<float> spectrumMagnitudes;
    std::vector<float> spectrumFrequencies;
    SpectrumDrawStyle drawStyle;
    bool enabled;
    std::mutex dataMutex;
    
    void drawGrid(CDrawContext* context);
    void drawSpectrum(CDrawContext* context);
    void drawLines(CDrawContext* context);
    void drawDots(CDrawContext* context);
    void drawBins(CDrawContext* context);
    void drawFills(CDrawContext* context);
    CPoint frequencyToPosition(float freq, float magnitude);
    bool isFrequencyInRange(float freq);
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
    
    // Parameter update methods
    void updateParameter(int paramId, double value);
    
    // Color scheme (public for EQ view access)
    static const CColor kBackgroundColor;
    static const CColor kPanelColor;
    static const CColor kAccentColor;
    static const CColor kTextColor;
    static const CColor kGridColor;
    
private:
    CFrame* frame;
    bool isCreated;
    bool isVisible;
    uint32_t currentWidth;
    uint32_t currentHeight;
    const clap_host_t* host;
    
    // GUI components
    CViewContainer* leftPanel;
    CViewContainer* rightPanel;
    CViewContainer* eqPanel;
    CViewContainer* spectrumPanel;
    EQVisualizationView* eqView;
    SpectrumAnalyzerView* spectrumView;
    
    // Control references
    CKnob* cutoffKnob;
    CKnob* resonanceKnob;
    CKnob* driveKnob;
    CKnob* outputKnob;
    CSlider* mixSlider;
    CTextButton* bypassButton;
    COptionMenu* presetMenu;
    CTextLabel* brandLabel;
    CTextLabel* statusLabel;
    COptionMenu* spectrumStyleMenu;
    CTextButton* spectrumToggleButton;
    
    // Parameter storage
    double currentParams[PARAM_COUNT]; // Parameter array
    
    void createControls();
    void createLeftPanel();
    void createRightPanel();
    void createEQPanel();
    void createSpectrumPanel();
    void createBrandHeader();
    void styleControl(CView* control);
    void onParameterChanged(int paramId, double value);
    void updateSpectrumDisplay();
};