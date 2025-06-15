#include "my_plugin_gui.h"
#include "my_plugin.h"
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
#include <vstgui/lib/cdrawcontext.h>
#include <iostream>
#include <cmath>

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
    , currentWidth(800)
    , currentHeight(600)
    , host(host)
    , plugin(nullptr)
    , depthKnob(nullptr)
    , sharpnessKnob(nullptr)
    , selectivityKnob(nullptr)
    , balanceKnob(nullptr)
    , frequencyKnob(nullptr)
    , gainKnob(nullptr)
    , qKnob(nullptr)
    , modeButton(nullptr)
    , linkButton(nullptr)
    , bypassButton(nullptr)
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
            // Set a dark background color typical of modern audio plugins
            frame->setBackgroundColor(CColor(40, 42, 45, 255)); // Dark gray background
            createSoothe2StyleControls();
            isCreated = true;
            std::cout << "MyPlugin GUI: Created successfully" << std::endl;
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

void MyPluginEditor::valueChanged(CControl* control) {
    if (!plugin) return;
    
    // Determine which parameter was changed and update plugin
    uint32_t param_id = UINT32_MAX;
    double value = control->getValue();
    
    if (control == depthKnob) param_id = PARAM_DEPTH;
    else if (control == sharpnessKnob) param_id = PARAM_SHARPNESS;
    else if (control == selectivityKnob) param_id = PARAM_SELECTIVITY;
    else if (control == balanceKnob) param_id = PARAM_BALANCE;
    else if (control == frequencyKnob) param_id = PARAM_FREQUENCY;
    else if (control == gainKnob) param_id = PARAM_GAIN;
    else if (control == qKnob) param_id = PARAM_Q;
    else if (control == modeButton) param_id = PARAM_MODE;
    else if (control == linkButton) param_id = PARAM_LINK;
    else if (control == bypassButton) param_id = PARAM_BYPASS;
    
    if (param_id < PARAM_COUNT) {
        // Convert normalized value to parameter range
        const clap_param_info_t& info = param_infos[param_id];
        double param_value = info.min_value + value * (info.max_value - info.min_value);
        
        // Update plugin parameter
        plugin->param_values[param_id] = param_value;
        
        // Notify host of parameter change
        if (host) {
            auto params_host = (const clap_host_params_t*)host->get_extension(host, CLAP_EXT_PARAMS);
            if (params_host) {
                params_host->rescan(host, CLAP_PARAM_RESCAN_VALUES);
            }
        }
        
        std::cout << "MyPlugin GUI: Parameter " << param_id << " changed to " << param_value << std::endl;
    }
}

void MyPluginEditor::updateParameter(uint32_t param_id, double value) {
    if (param_id >= PARAM_COUNT) return;
    
    // Convert parameter value to normalized control value
    const clap_param_info_t& info = param_infos[param_id];
    float normalized = (value - info.min_value) / (info.max_value - info.min_value);
    
    // Update the appropriate control
    CControl* control = nullptr;
    switch (param_id) {
        case PARAM_DEPTH: control = depthKnob; break;
        case PARAM_SHARPNESS: control = sharpnessKnob; break;
        case PARAM_SELECTIVITY: control = selectivityKnob; break;
        case PARAM_BALANCE: control = balanceKnob; break;
        case PARAM_FREQUENCY: control = frequencyKnob; break;
        case PARAM_GAIN: control = gainKnob; break;
        case PARAM_Q: control = qKnob; break;
        case PARAM_MODE: control = modeButton; break;
        case PARAM_LINK: control = linkButton; break;
        case PARAM_BYPASS: control = bypassButton; break;
    }
    
    if (control) {
        control->setValue(normalized);
        control->invalid();
    }
}

void MyPluginEditor::createSoothe2StyleControls() {
    if (!frame) {
        return;
    }
    
    // === Header ===
    CRect titleRect(10, 10, 790, 40);
    auto titleLabel = new CTextLabel(titleRect, "Soothe2-Style Dynamic Resonance Suppressor");
    titleLabel->setFontColor(CColor(220, 220, 220, 255)); // Light gray text
    titleLabel->setBackColor(CColor(60, 62, 65, 255)); // Slightly lighter background
    titleLabel->setHoriAlign(kCenterText);
    titleLabel->setFont(kSystemFont);
    frame->addView(titleLabel);
    
    // === Left Panel - Main Controls ===
    
    // Main control section background
    CRect leftPanelRect(20, 60, 380, 560);
    auto leftPanel = new CViewContainer(leftPanelRect);
    leftPanel->setBackgroundColor(CColor(50, 52, 55, 255));
    frame->addView(leftPanel);
    
    // Depth control
    CRect depthKnobRect(30, 20, 80, 70);
    depthKnob = new CKnob(depthKnobRect, this, PARAM_DEPTH, nullptr, nullptr);
    depthKnob->setDefaultValue(param_infos[PARAM_DEPTH].default_value / (param_infos[PARAM_DEPTH].max_value - param_infos[PARAM_DEPTH].min_value));
    depthKnob->setValue(depthKnob->getDefaultValue());
    depthKnob->setColorShadowHandle(CColor(80, 80, 80, 255));
    depthKnob->setColorHandle(CColor(100, 150, 255, 255)); // Blue handle
    leftPanel->addView(depthKnob);
    
    CRect depthLabelRect(20, 75, 90, 95);
    auto depthLabel = new CTextLabel(depthLabelRect, "Depth");
    depthLabel->setFontColor(CColor(200, 200, 200, 255));
    depthLabel->setBackColor(CColor(0, 0, 0, 0)); // Transparent
    depthLabel->setHoriAlign(kCenterText);
    leftPanel->addView(depthLabel);
    
    // Sharpness control
    CRect sharpnessKnobRect(110, 20, 160, 70);
    sharpnessKnob = new CKnob(sharpnessKnobRect, this, PARAM_SHARPNESS, nullptr, nullptr);
    sharpnessKnob->setDefaultValue(param_infos[PARAM_SHARPNESS].default_value / (param_infos[PARAM_SHARPNESS].max_value - param_infos[PARAM_SHARPNESS].min_value));
    sharpnessKnob->setValue(sharpnessKnob->getDefaultValue());
    sharpnessKnob->setColorShadowHandle(CColor(80, 80, 80, 255));
    sharpnessKnob->setColorHandle(CColor(255, 150, 100, 255)); // Orange handle
    leftPanel->addView(sharpnessKnob);
    
    CRect sharpnessLabelRect(100, 75, 170, 95);
    auto sharpnessLabel = new CTextLabel(sharpnessLabelRect, "Sharpness");
    sharpnessLabel->setFontColor(CColor(200, 200, 200, 255));
    sharpnessLabel->setBackColor(CColor(0, 0, 0, 0));
    sharpnessLabel->setHoriAlign(kCenterText);
    leftPanel->addView(sharpnessLabel);
    
    // Selectivity control
    CRect selectivityKnobRect(190, 20, 240, 70);
    selectivityKnob = new CKnob(selectivityKnobRect, this, PARAM_SELECTIVITY, nullptr, nullptr);
    selectivityKnob->setDefaultValue(param_infos[PARAM_SELECTIVITY].default_value / (param_infos[PARAM_SELECTIVITY].max_value - param_infos[PARAM_SELECTIVITY].min_value));
    selectivityKnob->setValue(selectivityKnob->getDefaultValue());
    selectivityKnob->setColorShadowHandle(CColor(80, 80, 80, 255));
    selectivityKnob->setColorHandle(CColor(150, 255, 150, 255)); // Green handle
    leftPanel->addView(selectivityKnob);
    
    CRect selectivityLabelRect(180, 75, 250, 95);
    auto selectivityLabel = new CTextLabel(selectivityLabelRect, "Selectivity");
    selectivityLabel->setFontColor(CColor(200, 200, 200, 255));
    selectivityLabel->setBackColor(CColor(0, 0, 0, 0));
    selectivityLabel->setHoriAlign(kCenterText);
    leftPanel->addView(selectivityLabel);
    
    // Balance control
    CRect balanceKnobRect(270, 20, 320, 70);
    balanceKnob = new CKnob(balanceKnobRect, this, PARAM_BALANCE, nullptr, nullptr);
    balanceKnob->setDefaultValue((param_infos[PARAM_BALANCE].default_value - param_infos[PARAM_BALANCE].min_value) / (param_infos[PARAM_BALANCE].max_value - param_infos[PARAM_BALANCE].min_value));
    balanceKnob->setValue(balanceKnob->getDefaultValue());
    balanceKnob->setColorShadowHandle(CColor(80, 80, 80, 255));
    balanceKnob->setColorHandle(CColor(255, 255, 100, 255)); // Yellow handle
    leftPanel->addView(balanceKnob);
    
    CRect balanceLabelRect(260, 75, 330, 95);
    auto balanceLabel = new CTextLabel(balanceLabelRect, "Balance");
    balanceLabel->setFontColor(CColor(200, 200, 200, 255));
    balanceLabel->setBackColor(CColor(0, 0, 0, 0));
    balanceLabel->setHoriAlign(kCenterText);
    leftPanel->addView(balanceLabel);
    
    // Mode selection
    CRect modeRect(30, 120, 180, 145);
    modeButton = new CSegmentButton(modeRect, this, PARAM_MODE);
    CSegmentButton::Segment monoSeg;
    monoSeg.name = "Mono";
    modeButton->addSegment(monoSeg);
    CSegmentButton::Segment stereoSeg;
    stereoSeg.name = "Stereo";
    modeButton->addSegment(stereoSeg);
    CSegmentButton::Segment msSeg;
    msSeg.name = "Mid/Side";
    modeButton->addSegment(msSeg);
    modeButton->setSelectedSegment(1); // Default to Stereo
    leftPanel->addView(modeButton);
    
    CRect modeLabelRect(30, 150, 180, 170);
    auto modeLabel = new CTextLabel(modeLabelRect, "Processing Mode");
    modeLabel->setFontColor(CColor(200, 200, 200, 255));
    modeLabel->setBackColor(CColor(0, 0, 0, 0));
    leftPanel->addView(modeLabel);
    
    // Link and Bypass buttons
    CRect linkRect(200, 120, 250, 145);
    linkButton = new COnOffButton(linkRect, this, PARAM_LINK, nullptr, 0);
    linkButton->setValue(param_infos[PARAM_LINK].default_value);
    leftPanel->addView(linkButton);
    
    CRect linkLabelRect(200, 150, 250, 170);
    auto linkLabel = new CTextLabel(linkLabelRect, "Link");
    linkLabel->setFontColor(CColor(200, 200, 200, 255));
    linkLabel->setBackColor(CColor(0, 0, 0, 0));
    linkLabel->setHoriAlign(kCenterText);
    leftPanel->addView(linkLabel);
    
    CRect bypassRect(270, 120, 320, 145);
    bypassButton = new COnOffButton(bypassRect, this, PARAM_BYPASS, nullptr, 0);
    bypassButton->setValue(param_infos[PARAM_BYPASS].default_value);
    leftPanel->addView(bypassButton);
    
    CRect bypassLabelRect(270, 150, 320, 170);
    auto bypassLabel = new CTextLabel(bypassLabelRect, "Bypass");
    bypassLabel->setFontColor(CColor(200, 200, 200, 255));
    bypassLabel->setBackColor(CColor(0, 0, 0, 0));
    bypassLabel->setHoriAlign(kCenterText);
    leftPanel->addView(bypassLabel);
    
    // Filter controls section
    CRect filterSectionRect(30, 190, 320, 220);
    auto filterSectionLabel = new CTextLabel(filterSectionRect, "Filter Parameters");
    filterSectionLabel->setFontColor(CColor(180, 180, 180, 255));
    filterSectionLabel->setBackColor(CColor(60, 62, 65, 255));
    filterSectionLabel->setHoriAlign(kCenterText);
    leftPanel->addView(filterSectionLabel);
    
    // Frequency control
    CRect frequencyKnobRect(30, 240, 80, 290);
    frequencyKnob = new CKnob(frequencyKnobRect, this, PARAM_FREQUENCY, nullptr, nullptr);
    frequencyKnob->setDefaultValue((param_infos[PARAM_FREQUENCY].default_value - param_infos[PARAM_FREQUENCY].min_value) / (param_infos[PARAM_FREQUENCY].max_value - param_infos[PARAM_FREQUENCY].min_value));
    frequencyKnob->setValue(frequencyKnob->getDefaultValue());
    frequencyKnob->setColorShadowHandle(CColor(80, 80, 80, 255));
    frequencyKnob->setColorHandle(CColor(255, 100, 150, 255)); // Pink handle
    leftPanel->addView(frequencyKnob);
    
    CRect frequencyLabelRect(20, 295, 90, 315);
    auto frequencyLabel = new CTextLabel(frequencyLabelRect, "Frequency");
    frequencyLabel->setFontColor(CColor(200, 200, 200, 255));
    frequencyLabel->setBackColor(CColor(0, 0, 0, 0));
    frequencyLabel->setHoriAlign(kCenterText);
    leftPanel->addView(frequencyLabel);
    
    // Gain control
    CRect gainKnobRect(125, 240, 175, 290);
    gainKnob = new CKnob(gainKnobRect, this, PARAM_GAIN, nullptr, nullptr);
    gainKnob->setDefaultValue((param_infos[PARAM_GAIN].default_value - param_infos[PARAM_GAIN].min_value) / (param_infos[PARAM_GAIN].max_value - param_infos[PARAM_GAIN].min_value));
    gainKnob->setValue(gainKnob->getDefaultValue());
    gainKnob->setColorShadowHandle(CColor(80, 80, 80, 255));
    gainKnob->setColorHandle(CColor(150, 100, 255, 255)); // Purple handle
    leftPanel->addView(gainKnob);
    
    CRect gainLabelRect(115, 295, 185, 315);
    auto gainLabel = new CTextLabel(gainLabelRect, "Gain");
    gainLabel->setFontColor(CColor(200, 200, 200, 255));
    gainLabel->setBackColor(CColor(0, 0, 0, 0));
    gainLabel->setHoriAlign(kCenterText);
    leftPanel->addView(gainLabel);
    
    // Q control
    CRect qKnobRect(220, 240, 270, 290);
    qKnob = new CKnob(qKnobRect, this, PARAM_Q, nullptr, nullptr);
    qKnob->setDefaultValue((param_infos[PARAM_Q].default_value - param_infos[PARAM_Q].min_value) / (param_infos[PARAM_Q].max_value - param_infos[PARAM_Q].min_value));
    qKnob->setValue(qKnob->getDefaultValue());
    qKnob->setColorShadowHandle(CColor(80, 80, 80, 255));
    qKnob->setColorHandle(CColor(100, 255, 200, 255)); // Cyan handle
    leftPanel->addView(qKnob);
    
    CRect qLabelRect(210, 295, 280, 315);
    auto qLabel = new CTextLabel(qLabelRect, "Q");
    qLabel->setFontColor(CColor(200, 200, 200, 255));
    qLabel->setBackColor(CColor(0, 0, 0, 0));
    qLabel->setHoriAlign(kCenterText);
    leftPanel->addView(qLabel);
    
    // === Right Panel - Spectrum and Curve Display ===
    
    // Spectrum display area
    CRect rightPanelRect(400, 60, 780, 560);
    auto rightPanel = new CViewContainer(rightPanelRect);
    rightPanel->setBackgroundColor(CColor(35, 37, 40, 255)); // Darker background for display area
    frame->addView(rightPanel);
    
    // Spectrum display title
    CRect spectrumTitleRect(10, 10, 370, 35);
    auto spectrumTitle = new CTextLabel(spectrumTitleRect, "Spectrum & Filter Response");
    spectrumTitle->setFontColor(CColor(220, 220, 220, 255));
    spectrumTitle->setBackColor(CColor(0, 0, 0, 0));
    spectrumTitle->setHoriAlign(kCenterText);
    rightPanel->addView(spectrumTitle);
    
    // Create a visual representation of spectrum analyzer and filter curve
    // This is a placeholder - in a real implementation you'd have custom views
    CRect spectrumRect(20, 50, 360, 350);
    auto spectrumView = new CViewContainer(spectrumRect);
    spectrumView->setBackgroundColor(CColor(25, 27, 30, 255)); // Very dark background
    rightPanel->addView(spectrumView);
    
    // Add some labels for frequency markers
    const char* freqLabels[] = {"20Hz", "100Hz", "1kHz", "10kHz", "20kHz"};
    for (int i = 0; i < 5; i++) {
        CRect freqLabelRect(20 + i * 68, 360, 88 + i * 68, 380);
        auto freqLabel = new CTextLabel(freqLabelRect, freqLabels[i]);
        freqLabel->setFontColor(CColor(150, 150, 150, 255));
        freqLabel->setBackColor(CColor(0, 0, 0, 0));
        freqLabel->setHoriAlign(kCenterText);
        rightPanel->addView(freqLabel);
    }
    
    // Control points area
    CRect controlsRect(20, 400, 360, 480);
    auto controlsView = new CViewContainer(controlsRect);
    controlsView->setBackgroundColor(CColor(45, 47, 50, 255));
    rightPanel->addView(controlsView);
    
    CRect controlsTitleRect(10, 385, 370, 405);
    auto controlsTitle = new CTextLabel(controlsTitleRect, "Control Points & Settings");
    controlsTitle->setFontColor(CColor(200, 200, 200, 255));
    controlsTitle->setBackColor(CColor(0, 0, 0, 0));
    controlsTitle->setHoriAlign(kCenterText);
    rightPanel->addView(controlsTitle);
    
    // Status bar
    CRect statusRect(10, 570, 790, 590);
    auto statusLabel = new CTextLabel(statusRect, "Soothe2-Style GUI - Professional Audio Plugin Interface");
    statusLabel->setFontColor(CColor(120, 140, 160, 255));
    statusLabel->setBackColor(CColor(55, 57, 60, 255));
    statusLabel->setHoriAlign(kCenterText);
    frame->addView(statusLabel);
}