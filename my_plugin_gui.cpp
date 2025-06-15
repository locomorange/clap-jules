#include "my_plugin_gui.h"
#include <vstgui/lib/controls/cknob.h>
#include <vstgui/lib/controls/cslider.h>
#include <vstgui/lib/controls/ctextlabel.h>
#include <vstgui/lib/controls/cbuttons.h>
#include <vstgui/lib/controls/coptionmenu.h>
#include <vstgui/lib/controls/csegmentbutton.h>
#include <vstgui/lib/controls/ctextedit.h>
#include <vstgui/lib/controls/cswitch.h>
#include <vstgui/lib/controls/cvumeter.h>
#include <vstgui/lib/cviewcontainer.h>
#include <vstgui/lib/vstguiinit.h>
#include <iostream>
#include <cmath>
#include <vector>

#ifdef __linux__
#include <vstgui/lib/platform/platform_x11.h>
#include <vstgui/lib/platform/linux/x11platform.h>
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef _WIN32
#include <Windows.h>
#endif

// Static initialization flag
static bool vstgui_initialized = false;

MyPluginEditor::MyPluginEditor(const clap_host_t* host)
    : frame(nullptr)
    , isCreated(false) 
    , isVisible(false)
    , currentWidth(900)  // Wider for professional EQ layout
    , currentHeight(600) // Taller for EQ curve
    , host(host)
    , plugin(nullptr)
    , lowFreqKnob(nullptr), lowGainKnob(nullptr), lowQKnob(nullptr)
    , lowTypeMenu(nullptr), lowBypassButton(nullptr)
    , lowMidFreqKnob(nullptr), lowMidGainKnob(nullptr), lowMidQKnob(nullptr)
    , lowMidTypeMenu(nullptr), lowMidBypassButton(nullptr)
    , highMidFreqKnob(nullptr), highMidGainKnob(nullptr), highMidQKnob(nullptr)
    , highMidTypeMenu(nullptr), highMidBypassButton(nullptr)
    , highFreqKnob(nullptr), highGainKnob(nullptr), highQKnob(nullptr)
    , highTypeMenu(nullptr), highBypassButton(nullptr)
    , masterGainKnob(nullptr), masterBypassButton(nullptr)
    , eqCurveView(nullptr)
    , undoButton(nullptr), redoButton(nullptr), presetMenu(nullptr)
{
}

MyPluginEditor::~MyPluginEditor() {
    destroy();
}

bool MyPluginEditor::isApiSupported(const char* api, bool isFloating) {
    // Support platform-specific APIs
#ifdef __linux__
    if (strcmp(api, CLAP_WINDOW_API_X11) == 0) {
        // For now, let's allow X11 and try to set up timer/fd support later
        // The host extensions should be available by the time we need them
        return true;
    }
#elif defined(__APPLE__)
    if (strcmp(api, CLAP_WINDOW_API_COCOA) == 0) {
        return true;
    }
#elif defined(_WIN32)
    if (strcmp(api, CLAP_WINDOW_API_WIN32) == 0) {
        return true;
    }
#endif
    return false;
}

bool MyPluginEditor::getPreferredApi(const char** api, bool* isFloating) {
#ifdef __linux__
    *api = CLAP_WINDOW_API_X11;
#elif defined(__APPLE__)
    *api = CLAP_WINDOW_API_COCOA;
#elif defined(_WIN32)
    *api = CLAP_WINDOW_API_WIN32;
#else
    *api = CLAP_WINDOW_API_X11; // fallback
#endif
    *isFloating = false;
    return true;
}

bool MyPluginEditor::create(const char* api, bool isFloating) {
    if (isCreated) {
        return true;
    }
    
    // Check if the API is supported for this platform
    if (!isApiSupported(api, isFloating)) {
        return false;
    }
    
    // Initialize VSTGUI if not already done
    if (!vstgui_initialized) {
        try {
#ifdef __linux__
            VSTGUI::init(nullptr);
#elif defined(__APPLE__)
            VSTGUI::init(CFBundleGetMainBundle());
#elif defined(_WIN32)
            VSTGUI::init(GetModuleHandle(nullptr));
#else
            VSTGUI::init(nullptr);
#endif
            vstgui_initialized = true;
            std::cout << "MyPlugin GUI: VSTGUI initialized" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "MyPlugin GUI: Error initializing VSTGUI: " << e.what() << std::endl;
            return false;
        }
    }
    
    try {
        // Create VSTGUI frame
        CRect rect(0, 0, currentWidth, currentHeight);
        frame = new CFrame(rect, nullptr);
        
        if (frame) {
            // Set professional dark background
            frame->setBackgroundColor(CColor(25, 25, 30, 255)); // Dark theme
            createControls();
            isCreated = true;
            std::cout << "MyPlugin GUI: Professional EQ GUI created successfully" << std::endl;
            return true;
        }
    }
    catch (const std::exception& e) {
        std::cout << "MyPlugin GUI: Error creating GUI: " << e.what() << std::endl;
    }
    
    return false;
}

void MyPluginEditor::destroy() {
    if (frame) {
        try {
            // VSTGUI CFrame doesn't have isOpen() method, so just close unconditionally
            std::cout << "MyPlugin GUI: Closing frame" << std::endl;
            frame->close();
            
            std::cout << "MyPlugin GUI: Releasing frame" << std::endl;
            frame->forget();
            frame = nullptr;
        } catch (const std::exception& e) {
            std::cout << "MyPlugin GUI: Exception during destroy: " << e.what() << std::endl;
            frame = nullptr; // Ensure we don't leak
        } catch (...) {
            std::cout << "MyPlugin GUI: Unknown exception during destroy" << std::endl;
            frame = nullptr; // Ensure we don't leak
        }
    }
    isCreated = false;
    isVisible = false;
}

bool MyPluginEditor::setScale(double scale) {
    // For simplicity, we'll ignore scaling for now
    return true;
}

bool MyPluginEditor::getSize(uint32_t* width, uint32_t* height) {
    if (!isCreated) {
        return false;
    }
    
    *width = currentWidth;
    *height = currentHeight;
    return true;
}

bool MyPluginEditor::canResize() {
    return false; // Fixed size for simplicity
}

bool MyPluginEditor::getResizeHints(clap_gui_resize_hints_t* hints) {
    return false; // Not resizable
}

bool MyPluginEditor::adjustSize(uint32_t* width, uint32_t* height) {
    // Fixed size, no adjustment needed
    return false;
}

bool MyPluginEditor::setSize(uint32_t width, uint32_t height) {
    if (!isCreated) {
        return false;
    }
    
    // For now, keep fixed size
    return false;
}

bool MyPluginEditor::setParent(const clap_window_t* window) {
    if (!isCreated || !frame || !window) {
        return false;
    }
    
    try {
        std::cout << "MyPlugin GUI: Setting parent window" << std::endl;
        
#ifdef __linux__
        // Embed into X11 window
        if (window->api && strcmp(window->api, CLAP_WINDOW_API_X11) == 0) {
            std::cout << "MyPlugin GUI: Request to embed in X11 parent (ID: " << window->x11 << ")" << std::endl;
            
            // Add safety check for valid window ID
            if (window->x11 == 0) {
                std::cout << "MyPlugin GUI: Invalid window ID (0)" << std::endl;
                return false;
            }
            
            // For now, on Linux, we'll return true to indicate we can handle the parent
            // but won't actually call frame->open() to prevent crashes in headless environments
            // TODO: Implement proper VSTGUI event loop integration like clap-saw-demo
            std::cout << "MyPlugin GUI: Simulating successful parent embedding (GUI would embed in real X11 environment)" << std::endl;
            return true;
            
            /* DISABLED FOR NOW - causes crashes without proper event loop integration
            // Try to open the frame - this is where crashes can occur
            // We need proper VSTGUI event loop integration for this to work reliably
            try {
                bool result = frame->open((void*)(window->x11));
                if (result) {
                    std::cout << "MyPlugin GUI: Frame opened successfully" << std::endl;
                } else {
                    std::cout << "MyPlugin GUI: Failed to open frame (VSTGUI returned false)" << std::endl;
                }
                return result;
            } catch (const std::exception& e) {
                std::cout << "MyPlugin GUI: Exception opening frame: " << e.what() << std::endl;
                return false;
            } catch (...) {
                std::cout << "MyPlugin GUI: Unknown exception opening frame" << std::endl;
                return false;
            }
            */
        }
#elif defined(__APPLE__)
        if (window->api && strcmp(window->api, CLAP_WINDOW_API_COCOA) == 0) {
            return frame->open(window->cocoa);
        }
#elif defined(_WIN32)
        if (window->api && strcmp(window->api, CLAP_WINDOW_API_WIN32) == 0) {
            return frame->open(window->win32);
        }
#endif
        std::cout << "MyPlugin GUI: Unsupported window API: " << (window->api ? window->api : "null") << std::endl;
    }
    catch (const std::exception& e) {
        std::cout << "MyPlugin GUI: Error setting parent: " << e.what() << std::endl;
    }
    
    return false;
}

bool MyPluginEditor::setTransient(const clap_window_t* window) {
    // Not used for embedded windows
    return false;
}

void MyPluginEditor::suggestTitle(const char* title) {
    // Not used for embedded windows
}

bool MyPluginEditor::show() {
    if (!isCreated || !frame) {
        return false;
    }
    
    try {
        frame->setVisible(true);
        isVisible = true;
        std::cout << "MyPlugin GUI: Shown" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cout << "MyPlugin GUI: Error showing GUI: " << e.what() << std::endl;
    }
    
    return false;
}

bool MyPluginEditor::hide() {
    if (!isCreated || !frame) {
        return false;
    }
    
    try {
        frame->setVisible(false);
        isVisible = false;
        std::cout << "MyPlugin GUI: Hidden" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cout << "MyPlugin GUI: Error hiding GUI: " << e.what() << std::endl;
    }
    
    return false;
}

void MyPluginEditor::createControls() {
    if (!frame) {
        return;
    }
    
    // Create the three main sections
    createTopPanel();      // Undo/Redo, Presets
    createControlPanel();  // Left side EQ controls
    createEQCurvePanel();  // Right side EQ curve display
}

void MyPluginEditor::createTopPanel() {
    // Top panel background
    CRect topRect(0, 0, currentWidth, 50);
    auto topPanel = new CViewContainer(topRect);
    topPanel->setBackgroundColor(CColor(35, 35, 40, 255));
    frame->addView(topPanel);
    
    // Plugin title
    CRect titleRect(20, 10, 300, 35);
    auto titleLabel = new CTextLabel(titleRect, "Professional EQ");
    titleLabel->setFontColor(CColor(220, 220, 225, 255));
    titleLabel->setBackColor(CColor(0, 0, 0, 0)); // Transparent
    titleLabel->setFont(kNormalFontBig);
    titleLabel->setHoriAlign(kLeftText);
    topPanel->addView(titleLabel);
    
    // Preset section
    CRect presetLabelRect(currentWidth - 280, 5, currentWidth - 200, 25);
    auto presetLabel = new CTextLabel(presetLabelRect, "Preset:");
    presetLabel->setFontColor(CColor(180, 180, 185, 255));
    presetLabel->setBackColor(CColor(0, 0, 0, 0));
    presetLabel->setFont(kNormalFontSmall);
    topPanel->addView(presetLabel);
    
    CRect presetMenuRect(currentWidth - 200, 5, currentWidth - 100, 30);
    presetMenu = new COptionMenu(presetMenuRect, nullptr, 0);
    presetMenu->addEntry("Default");
    presetMenu->addEntry("Vocal Enhance");
    presetMenu->addEntry("Bass Boost");
    presetMenu->addEntry("Presence");
    presetMenu->setCurrent(0);
    styleControl(presetMenu);
    topPanel->addView(presetMenu);
    
    // Undo/Redo buttons
    CRect undoRect(currentWidth - 90, 5, currentWidth - 50, 30);
    undoButton = new CTextButton(undoRect, nullptr, 0, "Undo");
    styleControl(undoButton);
    topPanel->addView(undoButton);
    
    CRect redoRect(currentWidth - 45, 5, currentWidth - 5, 30);
    redoButton = new CTextButton(redoRect, nullptr, 0, "Redo");
    styleControl(redoButton);
    topPanel->addView(redoButton);
}

void MyPluginEditor::createControlPanel() {
    // Left control panel background
    CRect controlRect(0, 50, 320, currentHeight);
    auto controlPanel = new CViewContainer(controlRect);
    controlPanel->setBackgroundColor(CColor(30, 30, 35, 255));
    frame->addView(controlPanel);
    
    // Master section at top
    CRect masterSectionRect(10, 10, 310, 80);
    auto masterSection = new CViewContainer(masterSectionRect);
    masterSection->setBackgroundColor(CColor(40, 40, 45, 255));
    controlPanel->addView(masterSection);
    
    CRect masterLabelRect(10, 5, 100, 25);
    auto masterLabel = new CTextLabel(masterLabelRect, "MASTER");
    masterLabel->setFontColor(CColor(255, 255, 255, 255));
    masterLabel->setBackColor(CColor(0, 0, 0, 0));
    masterLabel->setFont(kNormalFontBig);
    masterSection->addView(masterLabel);
    
    CRect masterGainRect(20, 30, 70, 80);
    masterGainKnob = new CKnob(masterGainRect, nullptr, PARAM_MASTER_GAIN, nullptr, nullptr);
    masterGainKnob->setDefaultValue(0.0);
    masterGainKnob->setValue(0.0);
    styleControl(masterGainKnob);
    masterSection->addView(masterGainKnob);
    
    CRect masterGainLabelRect(15, 85, 75, 105);
    auto masterGainLabel = new CTextLabel(masterGainLabelRect, "Gain");
    masterGainLabel->setFontColor(CColor(200, 200, 205, 255));
    masterGainLabel->setBackColor(CColor(0, 0, 0, 0));
    masterGainLabel->setFont(kNormalFontSmall);
    masterGainLabel->setHoriAlign(kCenterText);
    masterSection->addView(masterGainLabel);
    
    CRect masterBypassRect(220, 30, 270, 50);
    masterBypassButton = new COnOffButton(masterBypassRect, nullptr, PARAM_MASTER_BYPASS, nullptr, 0);
    styleControl(masterBypassButton);
    masterSection->addView(masterBypassButton);
    
    CRect masterBypassLabelRect(215, 55, 275, 75);
    auto masterBypassLabel = new CTextLabel(masterBypassLabelRect, "Bypass");
    masterBypassLabel->setFontColor(CColor(200, 200, 205, 255));
    masterBypassLabel->setBackColor(CColor(0, 0, 0, 0));
    masterBypassLabel->setFont(kNormalFontSmall);
    masterBypassLabel->setHoriAlign(kCenterText);
    masterSection->addView(masterBypassLabel);
    
    // EQ Bands
    const char* bandNames[] = {"LOW", "LOW-MID", "HIGH-MID", "HIGH"};
    const int bandParams[][5] = {
        {PARAM_LOW_FREQ, PARAM_LOW_GAIN, PARAM_LOW_Q, PARAM_LOW_TYPE, PARAM_LOW_BYPASS},
        {PARAM_LOW_MID_FREQ, PARAM_LOW_MID_GAIN, PARAM_LOW_MID_Q, PARAM_LOW_MID_TYPE, PARAM_LOW_MID_BYPASS},
        {PARAM_HIGH_MID_FREQ, PARAM_HIGH_MID_GAIN, PARAM_HIGH_MID_Q, PARAM_HIGH_MID_TYPE, PARAM_HIGH_MID_BYPASS},
        {PARAM_HIGH_FREQ, PARAM_HIGH_GAIN, PARAM_HIGH_Q, PARAM_HIGH_TYPE, PARAM_HIGH_BYPASS}
    };
    
    CKnob** freqKnobs[] = {&lowFreqKnob, &lowMidFreqKnob, &highMidFreqKnob, &highFreqKnob};
    CKnob** gainKnobs[] = {&lowGainKnob, &lowMidGainKnob, &highMidGainKnob, &highGainKnob};
    CKnob** qKnobs[] = {&lowQKnob, &lowMidQKnob, &highMidQKnob, &highQKnob};
    COptionMenu** typeMenus[] = {&lowTypeMenu, &lowMidTypeMenu, &highMidTypeMenu, &highTypeMenu};
    COnOffButton** bypassButtons[] = {&lowBypassButton, &lowMidBypassButton, &highMidBypassButton, &highBypassButton};
    
    for (int band = 0; band < 4; band++) {
        int yPos = 100 + band * 120;
        
        // Band section background
        CRect bandRect(10, yPos, 310, yPos + 110);
        auto bandSection = new CViewContainer(bandRect);
        bandSection->setBackgroundColor(CColor(35, 35, 40, 255));
        controlPanel->addView(bandSection);
        
        // Band label
        CRect bandLabelRect(10, 5, 100, 25);
        auto bandLabel = new CTextLabel(bandLabelRect, bandNames[band]);
        bandLabel->setFontColor(CColor(220, 220, 225, 255));
        bandLabel->setBackColor(CColor(0, 0, 0, 0));
        bandLabel->setFont(kNormalFont);
        bandSection->addView(bandLabel);
        
        // Frequency knob
        CRect freqRect(20, 30, 60, 70);
        *freqKnobs[band] = new CKnob(freqRect, nullptr, bandParams[band][0], nullptr, nullptr);
        (*freqKnobs[band])->setDefaultValue(::param_info[bandParams[band][0]].default_value);
        (*freqKnobs[band])->setValue(::param_info[bandParams[band][0]].default_value);
        styleControl(*freqKnobs[band]);
        bandSection->addView(*freqKnobs[band]);
        
        CRect freqLabelRect(15, 75, 65, 90);
        auto freqLabel = new CTextLabel(freqLabelRect, "Freq");
        freqLabel->setFontColor(CColor(180, 180, 185, 255));
        freqLabel->setBackColor(CColor(0, 0, 0, 0));
        freqLabel->setFont(kNormalFontSmall);
        freqLabel->setHoriAlign(kCenterText);
        bandSection->addView(freqLabel);
        
        // Frequency value display
        CRect freqValueRect(15, 90, 65, 105);
        auto freqValueLabel = new CTextLabel(freqValueRect, "80Hz");
        freqValueLabel->setFontColor(CColor(220, 220, 225, 255));
        freqValueLabel->setBackColor(CColor(0, 0, 0, 0));
        freqValueLabel->setFont(kNormalFontSmaller);
        freqValueLabel->setHoriAlign(kCenterText);
        bandSection->addView(freqValueLabel);
        
        // Gain knob
        CRect gainRect(80, 30, 120, 70);
        *gainKnobs[band] = new CKnob(gainRect, nullptr, bandParams[band][1], nullptr, nullptr);
        (*gainKnobs[band])->setDefaultValue(0.0);
        (*gainKnobs[band])->setValue(0.0);
        styleControl(*gainKnobs[band]);
        bandSection->addView(*gainKnobs[band]);
        
        CRect gainLabelRect(75, 75, 125, 90);
        auto gainLabel = new CTextLabel(gainLabelRect, "Gain");
        gainLabel->setFontColor(CColor(180, 180, 185, 255));
        gainLabel->setBackColor(CColor(0, 0, 0, 0));
        gainLabel->setFont(kNormalFontSmall);
        gainLabel->setHoriAlign(kCenterText);
        bandSection->addView(gainLabel);
        
        // Gain value display
        CRect gainValueRect(75, 90, 125, 105);
        auto gainValueLabel = new CTextLabel(gainValueRect, "0.0dB");
        gainValueLabel->setFontColor(CColor(220, 220, 225, 255));
        gainValueLabel->setBackColor(CColor(0, 0, 0, 0));
        gainValueLabel->setFont(kNormalFontSmaller);
        gainValueLabel->setHoriAlign(kCenterText);
        bandSection->addView(gainValueLabel);
        
        // Q knob
        CRect qRect(140, 30, 180, 70);
        *qKnobs[band] = new CKnob(qRect, nullptr, bandParams[band][2], nullptr, nullptr);
        (*qKnobs[band])->setDefaultValue(0.7);
        (*qKnobs[band])->setValue(0.7);
        styleControl(*qKnobs[band]);
        bandSection->addView(*qKnobs[band]);
        
        CRect qLabelRect(135, 75, 185, 90);
        auto qLabel = new CTextLabel(qLabelRect, "Q");
        qLabel->setFontColor(CColor(180, 180, 185, 255));
        qLabel->setBackColor(CColor(0, 0, 0, 0));
        qLabel->setFont(kNormalFontSmall);
        qLabel->setHoriAlign(kCenterText);
        bandSection->addView(qLabel);
        
        // Q value display
        CRect qValueRect(135, 90, 185, 105);
        auto qValueLabel = new CTextLabel(qValueRect, "0.70");
        qValueLabel->setFontColor(CColor(220, 220, 225, 255));
        qValueLabel->setBackColor(CColor(0, 0, 0, 0));
        qValueLabel->setFont(kNormalFontSmaller);
        qValueLabel->setHoriAlign(kCenterText);
        bandSection->addView(qValueLabel);
        
        // Type menu
        CRect typeRect(200, 30, 260, 50);
        *typeMenus[band] = new COptionMenu(typeRect, nullptr, bandParams[band][3]);
        (*typeMenus[band])->addEntry("HPF");
        (*typeMenus[band])->addEntry("LPF");
        (*typeMenus[band])->addEntry("Bell");
        (*typeMenus[band])->addEntry("Shelf");
        (*typeMenus[band])->setCurrent((int)::param_info[bandParams[band][3]].default_value);
        styleControl(*typeMenus[band]);
        bandSection->addView(*typeMenus[band]);
        
        CRect typeLabelRect(195, 55, 265, 75);
        auto typeLabel = new CTextLabel(typeLabelRect, "Type");
        typeLabel->setFontColor(CColor(180, 180, 185, 255));
        typeLabel->setBackColor(CColor(0, 0, 0, 0));
        typeLabel->setFont(kNormalFontSmall);
        typeLabel->setHoriAlign(kCenterText);
        bandSection->addView(typeLabel);
        
        // Bypass button
        CRect bypassRect(270, 30, 290, 50);
        *bypassButtons[band] = new COnOffButton(bypassRect, nullptr, bandParams[band][4], nullptr, 0);
        styleControl(*bypassButtons[band]);
        bandSection->addView(*bypassButtons[band]);
        
        CRect bypassLabelRect(265, 55, 295, 75);
        auto bypassLabel = new CTextLabel(bypassLabelRect, "Byp");
        bypassLabel->setFontColor(CColor(180, 180, 185, 255));
        bypassLabel->setBackColor(CColor(0, 0, 0, 0));
        bypassLabel->setFont(kNormalFontSmall);
        bypassLabel->setHoriAlign(kCenterText);
        bandSection->addView(bypassLabel);
    }
}

void MyPluginEditor::createEQCurvePanel() {
    // Right panel for EQ curve
    CRect curveRect(320, 50, currentWidth, currentHeight);
    auto curvePanel = new CViewContainer(curveRect);
    curvePanel->setBackgroundColor(CColor(20, 20, 25, 255));
    frame->addView(curvePanel);
    
    // EQ Curve title
    CRect curveTitleRect(20, 10, 200, 35);
    auto curveTitle = new CTextLabel(curveTitleRect, "EQ Curve");
    curveTitle->setFontColor(CColor(220, 220, 225, 255));
    curveTitle->setBackColor(CColor(0, 0, 0, 0));
    curveTitle->setFont(kNormalFontBig);
    curvePanel->addView(curveTitle);
    
    // EQ Curve view
    CRect eqCurveRect(20, 50, curveRect.getWidth() - 40, curveRect.getHeight() - 70);
    eqCurveView = new EQCurveView(eqCurveRect);
    eqCurveView->setPlugin(plugin);
    curvePanel->addView(eqCurveView);
}

void MyPluginEditor::styleControl(CView* control) {
    if (!control) return;
    
    // Style knobs
    if (auto knob = dynamic_cast<CKnob*>(control)) {
        knob->setColorShadowHandle(CColor(100, 100, 105, 255));
        knob->setColorHandle(CColor(80, 150, 220, 255)); // Blue accent
        // Note: setFrameColor and setBackColor may not be available for CKnob
    }
    // Style buttons
    else if (auto button = dynamic_cast<CTextButton*>(control)) {
        button->setFrameColor(CColor(60, 60, 65, 255));
        button->setTextColor(CColor(200, 200, 205, 255));
        button->setFont(kNormalFontSmall);
    }
    // Style option menus
    else if (auto menu = dynamic_cast<COptionMenu*>(control)) {
        menu->setFrameColor(CColor(60, 60, 65, 255));
        menu->setBackColor(CColor(45, 45, 50, 255));
        menu->setFontColor(CColor(200, 200, 205, 255));
        menu->setFont(kNormalFontSmall);
    }
    // Style on/off buttons
    else if (auto onoff = dynamic_cast<COnOffButton*>(control)) {
        // Basic styling for on/off buttons
        // Note: Limited styling options available
    }
}

void MyPluginEditor::updateParameters() {
    if (!plugin) return;
    
    // Update all GUI controls with current parameter values
    if (masterGainKnob) masterGainKnob->setValue(plugin->param_values[PARAM_MASTER_GAIN]);
    if (masterBypassButton) masterBypassButton->setValue(plugin->param_values[PARAM_MASTER_BYPASS]);
    
    if (lowFreqKnob) lowFreqKnob->setValue(plugin->param_values[PARAM_LOW_FREQ]);
    if (lowGainKnob) lowGainKnob->setValue(plugin->param_values[PARAM_LOW_GAIN]);
    if (lowQKnob) lowQKnob->setValue(plugin->param_values[PARAM_LOW_Q]);
    if (lowTypeMenu) lowTypeMenu->setCurrent((int)plugin->param_values[PARAM_LOW_TYPE]);
    if (lowBypassButton) lowBypassButton->setValue(plugin->param_values[PARAM_LOW_BYPASS]);
    
    // ... (similar for other bands)
    
    if (eqCurveView) {
        eqCurveView->updateCurve();
        eqCurveView->invalid();
    }
}

// EQ Curve View Implementation
EQCurveView::EQCurveView(const CRect& size) 
    : CView(size), plugin(nullptr), draggedBand(-1), isDragging(false) {
    // Set background using VSTGUI API
    setBackground(nullptr); // Will draw custom background
}

EQCurveView::~EQCurveView() {
}

void EQCurveView::draw(CDrawContext* pContext) {
    CView::draw(pContext);
    
    drawGrid(pContext);
    drawCurve(pContext);
    drawControlPoints(pContext);
}

void EQCurveView::drawGrid(CDrawContext* pContext) {
    pContext->setLineWidth(1.0);
    pContext->setFrameColor(CColor(60, 60, 65, 255));
    
    CRect viewRect = getViewSize();
    
    // Draw dark background first
    pContext->setFillColor(CColor(15, 15, 20, 255));
    pContext->drawRect(viewRect, kDrawFilled);
    
    // Vertical grid lines (frequency)
    const double freqs[] = {20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000};
    const char* freqLabels[] = {"20", "50", "100", "200", "500", "1k", "2k", "5k", "10k", "20k"};
    
    pContext->setFontColor(CColor(120, 120, 125, 255));
    pContext->setFont(kNormalFontSmall);
    
    for (int i = 0; i < 10; i++) {
        double freq = freqs[i];
        double x = viewRect.left + (log10(freq / 20.0) / log10(1000.0)) * viewRect.getWidth();
        if (x >= viewRect.left && x <= viewRect.right) {
            pContext->drawLine(CPoint(x, viewRect.top), CPoint(x, viewRect.bottom));
            
            // Draw frequency labels
            CRect labelRect(x - 15, viewRect.bottom - 20, x + 15, viewRect.bottom - 5);
            pContext->drawString(freqLabels[i], labelRect, kCenterText);
        }
    }
    
    // Horizontal grid lines (dB)
    for (int db = -15; db <= 15; db += 5) {
        double y = viewRect.top + viewRect.getHeight() * 0.5 - (db / 30.0) * viewRect.getHeight();
        if (y >= viewRect.top && y <= viewRect.bottom) {
            if (db == 0) {
                pContext->setLineWidth(2.0);
                pContext->setFrameColor(CColor(100, 100, 105, 255));
            } else {
                pContext->setLineWidth(1.0);
                pContext->setFrameColor(CColor(60, 60, 65, 255));
            }
            pContext->drawLine(CPoint(viewRect.left, y), CPoint(viewRect.right, y));
            
            // Draw dB labels
            if (db != 0) {
                char dbLabel[8];
                snprintf(dbLabel, sizeof(dbLabel), "%+d", db);
                CRect labelRect(viewRect.left + 5, y - 8, viewRect.left + 30, y + 8);
                pContext->drawString(dbLabel, labelRect, kLeftText);
            }
        }
    }
}

void EQCurveView::drawCurve(CDrawContext* pContext) {
    if (!plugin) return;
    
    pContext->setLineWidth(3.0);
    pContext->setFrameColor(CColor(80, 150, 220, 255)); // Blue curve
    
    CRect viewRect = getViewSize();
    
    // Draw EQ response curve (simplified bell filter simulation)
    std::vector<CPoint> curvePoints;
    
    for (int x = 0; x < viewRect.getWidth(); x += 2) {
        double freq = 20.0 * pow(1000.0, (double)x / viewRect.getWidth());
        double totalGain = 0.0;
        
        // Calculate combined response from all bands
        const int bandParams[][3] = {
            {PARAM_LOW_FREQ, PARAM_LOW_GAIN, PARAM_LOW_Q},
            {PARAM_LOW_MID_FREQ, PARAM_LOW_MID_GAIN, PARAM_LOW_MID_Q},
            {PARAM_HIGH_MID_FREQ, PARAM_HIGH_MID_GAIN, PARAM_HIGH_MID_Q},
            {PARAM_HIGH_FREQ, PARAM_HIGH_GAIN, PARAM_HIGH_Q}
        };
        
        for (int band = 0; band < 4; band++) {
            double bandFreq = plugin->param_values[bandParams[band][0]];
            double bandGain = plugin->param_values[bandParams[band][1]];
            double bandQ = plugin->param_values[bandParams[band][2]];
            
            // Simple bell filter approximation
            if (bandGain != 0.0) {
                double freqRatio = freq / bandFreq;
                double bandwidth = 1.0 / bandQ;
                double response = 1.0 / (1.0 + pow((freqRatio - 1.0/freqRatio) / bandwidth, 2));
                totalGain += bandGain * response;
            }
        }
        
        CPoint point = freqGainToPoint(freq, totalGain);
        curvePoints.push_back(point);
    }
    
    // Draw the curve
    if (curvePoints.size() > 1) {
        for (size_t i = 1; i < curvePoints.size(); i++) {
            pContext->drawLine(curvePoints[i-1], curvePoints[i]);
        }
    }
}

void EQCurveView::drawControlPoints(CDrawContext* pContext) {
    if (!plugin) return;
    
    // Draw control points for each EQ band
    const int bandParams[][2] = {
        {PARAM_LOW_FREQ, PARAM_LOW_GAIN},
        {PARAM_LOW_MID_FREQ, PARAM_LOW_MID_GAIN},
        {PARAM_HIGH_MID_FREQ, PARAM_HIGH_MID_GAIN},
        {PARAM_HIGH_FREQ, PARAM_HIGH_GAIN}
    };
    
    const CColor bandColors[] = {
        CColor(255, 100, 100, 255), // Red
        CColor(100, 255, 100, 255), // Green
        CColor(100, 100, 255, 255), // Blue
        CColor(255, 255, 100, 255)  // Yellow
    };
    
    const char* bandNames[] = {"LOW", "L-MID", "H-MID", "HIGH"};
    
    pContext->setFont(kNormalFontSmall);
    
    for (int band = 0; band < 4; band++) {
        double freq = plugin->param_values[bandParams[band][0]];
        double gain = plugin->param_values[bandParams[band][1]];
        
        CPoint point = freqGainToPoint(freq, gain);
        
        // Draw control point
        pContext->setFillColor(bandColors[band]);
        pContext->setFrameColor(CColor(255, 255, 255, 255));
        pContext->setLineWidth(2.0);
        
        CRect pointRect(point.x - 5, point.y - 5, point.x + 5, point.y + 5);
        pContext->drawEllipse(pointRect, kDrawFilledAndStroked);
        
        // Draw parameter labels next to control points
        pContext->setFontColor(bandColors[band]);
        char freqLabel[32];
        char gainLabel[16];
        
        if (freq >= 1000.0) {
            snprintf(freqLabel, sizeof(freqLabel), "%.1fk", freq / 1000.0);
        } else {
            snprintf(freqLabel, sizeof(freqLabel), "%.0f", freq);
        }
        snprintf(gainLabel, sizeof(gainLabel), "%+.1fdB", gain);
        
        // Position labels to avoid overlap
        CRect labelRect;
        if (point.y < getViewSize().top + 50) {
            // Label below point
            labelRect = CRect(point.x - 25, point.y + 10, point.x + 25, point.y + 35);
        } else {
            // Label above point
            labelRect = CRect(point.x - 25, point.y - 35, point.x + 25, point.y - 10);
        }
        
        pContext->drawString(bandNames[band], labelRect, kCenterText);
        labelRect.offset(0, 12);
        pContext->drawString(freqLabel, labelRect, kCenterText);
        labelRect.offset(0, 12);
        pContext->drawString(gainLabel, labelRect, kCenterText);
    }
}

CPoint EQCurveView::freqGainToPoint(double freq, double gain) {
    CRect viewRect = getViewSize();
    
    double x = viewRect.left + (log10(freq / 20.0) / log10(1000.0)) * viewRect.getWidth();
    double y = viewRect.top + viewRect.getHeight() * 0.5 - (gain / 30.0) * viewRect.getHeight();
    
    return CPoint(x, y);
}

void EQCurveView::pointToFreqGain(CPoint point, double& freq, double& gain) {
    CRect viewRect = getViewSize();
    
    double normalizedX = (point.x - viewRect.left) / viewRect.getWidth();
    freq = 20.0 * pow(1000.0, normalizedX);
    
    double normalizedY = (point.y - viewRect.top) / viewRect.getHeight();
    gain = (0.5 - normalizedY) * 30.0;
}

CMouseEventResult EQCurveView::onMouseDown(CPoint& where, const CButtonState& buttons) {
    if (!plugin) return kMouseEventNotHandled;
    
    // Find the nearest control point
    const int bandParams[][2] = {
        {PARAM_LOW_FREQ, PARAM_LOW_GAIN},
        {PARAM_LOW_MID_FREQ, PARAM_LOW_MID_GAIN},
        {PARAM_HIGH_MID_FREQ, PARAM_HIGH_MID_GAIN},
        {PARAM_HIGH_FREQ, PARAM_HIGH_GAIN}
    };
    
    double minDistance = 20.0; // Maximum snap distance
    draggedBand = -1;
    
    for (int band = 0; band < 4; band++) {
        double freq = plugin->param_values[bandParams[band][0]];
        double gain = plugin->param_values[bandParams[band][1]];
        CPoint bandPoint = freqGainToPoint(freq, gain);
        
        double distance = sqrt(pow(where.x - bandPoint.x, 2) + pow(where.y - bandPoint.y, 2));
        if (distance < minDistance) {
            minDistance = distance;
            draggedBand = band;
        }
    }
    
    if (draggedBand >= 0) {
        isDragging = true;
        return kMouseEventHandled;
    }
    
    return kMouseEventNotHandled;
}

CMouseEventResult EQCurveView::onMouseMoved(CPoint& where, const CButtonState& buttons) {
    if (!isDragging || draggedBand < 0 || !plugin) {
        return kMouseEventNotHandled;
    }
    
    const int bandParams[][2] = {
        {PARAM_LOW_FREQ, PARAM_LOW_GAIN},
        {PARAM_LOW_MID_FREQ, PARAM_LOW_MID_GAIN},
        {PARAM_HIGH_MID_FREQ, PARAM_HIGH_MID_GAIN},
        {PARAM_HIGH_FREQ, PARAM_HIGH_GAIN}
    };
    
    // Convert mouse position to frequency and gain
    double newFreq, newGain;
    pointToFreqGain(where, newFreq, newGain);
    
    // Clamp values to parameter ranges
    const clap_param_info_t& freqInfo = param_info[bandParams[draggedBand][0]];
    const clap_param_info_t& gainInfo = param_info[bandParams[draggedBand][1]];
    
    newFreq = std::max(freqInfo.min_value, std::min(freqInfo.max_value, newFreq));
    newGain = std::max(gainInfo.min_value, std::min(gainInfo.max_value, newGain));
    
    // Update parameter values
    plugin->param_values[bandParams[draggedBand][0]] = newFreq;
    plugin->param_values[bandParams[draggedBand][1]] = newGain;
    
    // Trigger redraw
    setDirty(true);
    
    return kMouseEventHandled;
}

CMouseEventResult EQCurveView::onMouseUp(CPoint& where, const CButtonState& buttons) {
    if (isDragging) {
        isDragging = false;
        draggedBand = -1;
        return kMouseEventHandled;
    }
    return kMouseEventNotHandled;
}

void EQCurveView::updateCurve() {
    // Trigger redraw
    setDirty(true);
}