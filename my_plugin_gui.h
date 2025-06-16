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

// Custom spectrum visualization view
class SpectrumVisualizationView : public CView {
public:
    SpectrumVisualizationView(const CRect& size);
    ~SpectrumVisualizationView();
    
    void draw(CDrawContext* context) override;
    
    // Update spectrum data
    void updateSpectrumData(const std::vector<float>& spectrum_data, const std::vector<float>& frequency_bins);
    void setDrawingStyle(int style);
    
private:
    std::vector<float> spectrumData;
    std::vector<float> frequencyBins;
    int drawingStyle;
    uint64_t lastUpdateTime;
    
    void drawGrid(CDrawContext* context);
    void drawSpectrum(CDrawContext* context);
    void drawSpectrumAsLines(CDrawContext* context);
    void drawSpectrumAsDots(CDrawContext* context);
    void drawSpectrumAsBins(CDrawContext* context);
    void drawSpectrumAsFills(CDrawContext* context);
    CPoint frequencyToPosition(float freq, float magnitude);
    void updateTestData();
    
    // Constants for display
    static constexpr float MIN_FREQ = 20.0f;
    static constexpr float MAX_FREQ = 20000.0f;
    static constexpr float MIN_DB = -80.0f;
    static constexpr float MAX_DB = 20.0f;
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
    void updateSpectrumData(const std::vector<float>& spectrum_data, const std::vector<float>& frequency_bins);
    
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
    EQVisualizationView* eqView;
    SpectrumVisualizationView* spectrumView;
    
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
    
    // Parameter storage
    double currentParams[16]; // Simple parameter array
    
    void createControls();
    void createLeftPanel();
    void createRightPanel();
    void createBrandHeader();
    void styleControl(CView* control);
    void onParameterChanged(int paramId, double value);
};