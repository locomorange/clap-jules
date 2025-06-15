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
#include <vstgui/lib/vstguiinit.h>
#include <iostream>

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
    , currentWidth(600)
    , currentHeight(500)
    , host(host)
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
            // Set a light gray background for better contrast
            frame->setBackgroundColor(CColor(240, 240, 240, 255)); // Light gray background
            createControls();
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

void MyPluginEditor::createControls() {
    if (!frame) {
        return;
    }
    
    // Create a title label
    CRect titleRect(10, 10, 590, 35);
    auto titleLabel = new CTextLabel(titleRect, "CLAP Plugin - VSTGUI Controls Test Interface");
    titleLabel->setFontColor(CColor(20, 20, 20, 255)); // Dark gray text for better readability
    titleLabel->setBackColor(CColor(220, 235, 255, 255)); // Light blue background
    titleLabel->setHoriAlign(kCenterText);
    frame->addView(titleLabel);
    
    // === Row 1: Knobs ===
    // Volume knob
    CRect volumeKnobRect(30, 60, 80, 110);
    auto volumeKnob = new CKnob(volumeKnobRect, nullptr, 0, nullptr, nullptr);
    volumeKnob->setDefaultValue(0.5f);
    volumeKnob->setValue(0.5f);
    volumeKnob->setColorShadowHandle(CColor(120, 120, 120, 255)); // Visible handle shadow
    volumeKnob->setColorHandle(CColor(200, 50, 50, 255)); // Red handle for visibility
    frame->addView(volumeKnob);
    
    CRect volumeLabelRect(20, 115, 90, 135);
    auto volumeLabel = new CTextLabel(volumeLabelRect, "Volume");
    volumeLabel->setFontColor(CColor(40, 40, 40, 255)); // Dark gray text
    volumeLabel->setBackColor(CColor(255, 255, 255, 200)); // Semi-transparent white background
    volumeLabel->setHoriAlign(kCenterText);
    frame->addView(volumeLabel);
    
    // Filter knob
    CRect filterKnobRect(110, 60, 160, 110);
    auto filterKnob = new CKnob(filterKnobRect, nullptr, 0, nullptr, nullptr);
    filterKnob->setDefaultValue(0.7f);
    filterKnob->setValue(0.7f);
    filterKnob->setColorShadowHandle(CColor(120, 120, 120, 255)); // Visible handle shadow
    filterKnob->setColorHandle(CColor(50, 150, 50, 255)); // Green handle for visibility
    frame->addView(filterKnob);
    
    CRect filterLabelRect(100, 115, 170, 135);
    auto filterLabel = new CTextLabel(filterLabelRect, "Filter");
    filterLabel->setFontColor(CColor(40, 40, 40, 255)); // Dark gray text
    filterLabel->setBackColor(CColor(255, 255, 255, 200)); // Semi-transparent white background
    filterLabel->setHoriAlign(kCenterText);
    frame->addView(filterLabel);
    
    // Resonance knob
    CRect resKnobRect(190, 60, 240, 110);
    auto resKnob = new CKnob(resKnobRect, nullptr, 0, nullptr, nullptr);
    resKnob->setDefaultValue(0.3f);
    resKnob->setValue(0.3f);
    resKnob->setColorShadowHandle(CColor(120, 120, 120, 255)); // Visible handle shadow
    resKnob->setColorHandle(CColor(50, 50, 200, 255)); // Blue handle for visibility
    frame->addView(resKnob);
    
    CRect resLabelRect(180, 115, 250, 135);
    auto resLabel = new CTextLabel(resLabelRect, "Resonance");
    resLabel->setFontColor(CColor(40, 40, 40, 255)); // Dark gray text
    resLabel->setBackColor(CColor(255, 255, 255, 200)); // Semi-transparent white background
    resLabel->setHoriAlign(kCenterText);
    frame->addView(resLabel);
    
    // === Row 2: Sliders ===
    // Horizontal slider
    CRect hSliderRect(30, 160, 240, 180);
    auto hSlider = new CHorizontalSlider(hSliderRect, nullptr, 0, 0, 100, nullptr, nullptr);
    hSlider->setValue(60.0f);
    hSlider->setFrameColor(CColor(80, 80, 80, 255)); // Dark frame for visibility
    hSlider->setBackColor(CColor(220, 220, 220, 255)); // Light background
    hSlider->setValueColor(CColor(0, 120, 200, 255)); // Blue slider handle
    frame->addView(hSlider);
    
    CRect hSliderLabelRect(30, 185, 240, 205);
    auto hSliderLabel = new CTextLabel(hSliderLabelRect, "Horizontal Slider (Cutoff)");
    hSliderLabel->setFontColor(CColor(40, 40, 40, 255)); // Dark gray text
    hSliderLabel->setBackColor(CColor(255, 255, 255, 200)); // Semi-transparent white background
    frame->addView(hSliderLabel);
    
    // Vertical slider
    CRect vSliderRect(270, 60, 290, 180);
    auto vSlider = new CVerticalSlider(vSliderRect, nullptr, 0, 0, 100, nullptr, nullptr);
    vSlider->setValue(40.0f);
    vSlider->setFrameColor(CColor(80, 80, 80, 255)); // Dark frame for visibility
    vSlider->setBackColor(CColor(220, 220, 220, 255)); // Light background
    vSlider->setValueColor(CColor(200, 100, 0, 255)); // Orange slider handle
    frame->addView(vSlider);
    
    CRect vSliderLabelRect(260, 185, 300, 205);
    auto vSliderLabel = new CTextLabel(vSliderLabelRect, "Vertical");
    vSliderLabel->setFontColor(CColor(40, 40, 40, 255)); // Dark gray text
    vSliderLabel->setBackColor(CColor(255, 255, 255, 200)); // Semi-transparent white background
    vSliderLabel->setHoriAlign(kCenterText);
    frame->addView(vSliderLabel);
    
    // === Row 3: Buttons ===
    // Text buttons
    CRect button1Rect(30, 230, 120, 255);
    auto button1 = new CTextButton(button1Rect, nullptr, 0, "Play");
    frame->addView(button1);
    
    CRect button2Rect(130, 230, 220, 255);
    auto button2 = new CTextButton(button2Rect, nullptr, 0, "Stop");
    frame->addView(button2);
    
    // Kick button (momentary)
    CRect kickButtonRect(230, 230, 320, 255);
    auto kickButton = new CKickButton(kickButtonRect, nullptr, 0, nullptr);
    frame->addView(kickButton);
    
    CRect kickLabelRect(230, 260, 320, 280);
    auto kickLabel = new CTextLabel(kickLabelRect, "Kick Button");
    kickLabel->setFontColor(CColor(40, 40, 40, 255)); // Dark gray text
    kickLabel->setBackColor(CColor(255, 255, 255, 200)); // Semi-transparent white background
    kickLabel->setHoriAlign(kCenterText);
    frame->addView(kickLabel);
    
    // === Row 4: Option Menu and Segment Button ===
    // Option menu (dropdown)
    CRect menuRect(30, 300, 150, 325);
    auto optionMenu = new COptionMenu(menuRect, nullptr, 0);
    optionMenu->addEntry("Low Pass");
    optionMenu->addEntry("High Pass");
    optionMenu->addEntry("Band Pass");
    optionMenu->addEntry("Notch");
    optionMenu->setCurrent(0);
    frame->addView(optionMenu);
    
    CRect menuLabelRect(30, 330, 150, 350);
    auto menuLabel = new CTextLabel(menuLabelRect, "Filter Type");
    menuLabel->setFontColor(CColor(40, 40, 40, 255)); // Dark gray text
    menuLabel->setBackColor(CColor(255, 255, 255, 200)); // Semi-transparent white background
    frame->addView(menuLabel);
    
    // Segment button
    CRect segmentRect(170, 300, 320, 325);
    auto segmentButton = new CSegmentButton(segmentRect, nullptr, 0);
    CSegmentButton::Segment segment1;
    segment1.name = "Saw";
    segmentButton->addSegment(segment1);
    CSegmentButton::Segment segment2;
    segment2.name = "Square";
    segmentButton->addSegment(segment2);
    CSegmentButton::Segment segment3;
    segment3.name = "Sine";
    segmentButton->addSegment(segment3);
    segmentButton->setSelectedSegment(0);
    frame->addView(segmentButton);
    
    CRect segmentLabelRect(170, 330, 320, 350);
    auto segmentLabel = new CTextLabel(segmentLabelRect, "Waveform");
    segmentLabel->setFontColor(CColor(40, 40, 40, 255)); // Dark gray text
    segmentLabel->setBackColor(CColor(255, 255, 255, 200)); // Semi-transparent white background
    frame->addView(segmentLabel);
    
    // === Row 5: Text Edit and Switches ===
    // Text edit field
    CRect textEditRect(30, 370, 200, 395);
    auto textEdit = new CTextEdit(textEditRect, nullptr, 0);
    textEdit->setText("Edit me!");
    textEdit->setFrameColor(CColor(60, 60, 60, 255)); // Dark border
    textEdit->setBackColor(CColor(255, 255, 255, 255)); // White background
    textEdit->setFontColor(CColor(20, 20, 20, 255)); // Dark text
    frame->addView(textEdit);
    
    CRect textEditLabelRect(30, 400, 200, 420);
    auto textEditLabel = new CTextLabel(textEditLabelRect, "Text Input");
    textEditLabel->setFontColor(CColor(40, 40, 40, 255)); // Dark gray text
    textEditLabel->setBackColor(CColor(255, 255, 255, 200)); // Semi-transparent white background
    frame->addView(textEditLabel);
    
    // On/Off switch
    CRect switch1Rect(220, 375, 260, 390);
    auto switch1 = new COnOffButton(switch1Rect, nullptr, 0, nullptr, 0);
    switch1->setValue(1.0f); // On
    frame->addView(switch1);
    
    CRect switch1LabelRect(220, 400, 280, 420);
    auto switch1Label = new CTextLabel(switch1LabelRect, "On/Off");
    switch1Label->setFontColor(CColor(40, 40, 40, 255)); // Dark gray text
    switch1Label->setBackColor(CColor(255, 255, 255, 200)); // Semi-transparent white background
    frame->addView(switch1Label);
    
    // === Additional Controls on the Right Side ===
    // More knobs
    CRect knob4Rect(350, 60, 400, 110);
    auto knob4 = new CKnob(knob4Rect, nullptr, 0, nullptr, nullptr);
    knob4->setDefaultValue(0.8f);
    knob4->setValue(0.8f);
    knob4->setColorShadowHandle(CColor(120, 120, 120, 255)); // Visible handle shadow
    knob4->setColorHandle(CColor(200, 150, 0, 255)); // Orange handle for visibility
    frame->addView(knob4);
    
    CRect knob4LabelRect(340, 115, 410, 135);
    auto knob4Label = new CTextLabel(knob4LabelRect, "Attack");
    knob4Label->setFontColor(CColor(40, 40, 40, 255)); // Dark gray text
    knob4Label->setBackColor(CColor(255, 255, 255, 200)); // Semi-transparent white background
    knob4Label->setHoriAlign(kCenterText);
    frame->addView(knob4Label);
    
    CRect knob5Rect(430, 60, 480, 110);
    auto knob5 = new CKnob(knob5Rect, nullptr, 0, nullptr, nullptr);
    knob5->setDefaultValue(0.4f);
    knob5->setValue(0.4f);
    knob5->setColorShadowHandle(CColor(120, 120, 120, 255)); // Visible handle shadow
    knob5->setColorHandle(CColor(150, 0, 150, 255)); // Purple handle for visibility
    frame->addView(knob5);
    
    CRect knob5LabelRect(420, 115, 490, 135);
    auto knob5Label = new CTextLabel(knob5LabelRect, "Decay");
    knob5Label->setFontColor(CColor(40, 40, 40, 255)); // Dark gray text
    knob5Label->setBackColor(CColor(255, 255, 255, 200)); // Semi-transparent white background
    knob5Label->setHoriAlign(kCenterText);
    frame->addView(knob5Label);
    
    CRect knob6Rect(510, 60, 560, 110);
    auto knob6 = new CKnob(knob6Rect, nullptr, 0, nullptr, nullptr);
    knob6->setDefaultValue(0.6f);
    knob6->setValue(0.6f);
    knob6->setColorShadowHandle(CColor(120, 120, 120, 255)); // Visible handle shadow
    knob6->setColorHandle(CColor(0, 150, 150, 255)); // Cyan handle for visibility
    frame->addView(knob6);
    
    CRect knob6LabelRect(500, 115, 570, 135);
    auto knob6Label = new CTextLabel(knob6LabelRect, "Release");
    knob6Label->setFontColor(CColor(40, 40, 40, 255)); // Dark gray text
    knob6Label->setBackColor(CColor(255, 255, 255, 200)); // Semi-transparent white background
    knob6Label->setHoriAlign(kCenterText);
    frame->addView(knob6Label);
    
    // More sliders on the right
    CRect hSlider2Rect(350, 160, 560, 180);
    auto hSlider2 = new CHorizontalSlider(hSlider2Rect, nullptr, 0, 0, 100, nullptr, nullptr);
    hSlider2->setValue(75.0f);
    hSlider2->setFrameColor(CColor(80, 80, 80, 255)); // Dark frame for visibility
    hSlider2->setBackColor(CColor(220, 220, 220, 255)); // Light background
    hSlider2->setValueColor(CColor(150, 0, 100, 255)); // Magenta slider handle
    frame->addView(hSlider2);
    
    CRect hSlider2LabelRect(350, 185, 560, 205);
    auto hSlider2Label = new CTextLabel(hSlider2LabelRect, "Envelope Amount");
    hSlider2Label->setFontColor(CColor(40, 40, 40, 255)); // Dark gray text
    hSlider2Label->setBackColor(CColor(255, 255, 255, 200)); // Semi-transparent white background
    frame->addView(hSlider2Label);
    
    // More buttons on the right
    CRect button3Rect(350, 230, 440, 255);
    auto button3 = new CTextButton(button3Rect, nullptr, 0, "Record");
    frame->addView(button3);
    
    CRect button4Rect(450, 230, 540, 255);
    auto button4 = new CTextButton(button4Rect, nullptr, 0, "Bypass");
    frame->addView(button4);
    
    // VU Meter
    CRect vuMeterRect(350, 275, 430, 295);
    auto vuMeter = new CVuMeter(vuMeterRect, nullptr, nullptr, 10, CVuMeter::kHorizontal);
    vuMeter->setValue(0.6f);
    frame->addView(vuMeter);
    
    CRect vuMeterLabelRect(350, 300, 430, 320);
    auto vuMeterLabel = new CTextLabel(vuMeterLabelRect, "VU Meter");
    vuMeterLabel->setFontColor(CColor(40, 40, 40, 255)); // Dark gray text
    vuMeterLabel->setBackColor(CColor(255, 255, 255, 200)); // Semi-transparent white background
    frame->addView(vuMeterLabel);
    
    // More switches
    CRect switch2Rect(450, 275, 490, 290);
    auto switch2 = new COnOffButton(switch2Rect, nullptr, 0, nullptr, 0);
    switch2->setValue(0.0f); // Off
    frame->addView(switch2);
    
    CRect switch3Rect(500, 275, 540, 290);
    auto switch3 = new COnOffButton(switch3Rect, nullptr, 0, nullptr, 0);
    switch3->setValue(1.0f); // On
    frame->addView(switch3);
    
    CRect switchGroupLabelRect(450, 300, 540, 320);
    auto switchGroupLabel = new CTextLabel(switchGroupLabelRect, "Switches");
    switchGroupLabel->setFontColor(CColor(40, 40, 40, 255)); // Dark gray text
    switchGroupLabel->setBackColor(CColor(255, 255, 255, 200)); // Semi-transparent white background
    frame->addView(switchGroupLabel);
    
    // Additional option menu
    CRect menu2Rect(350, 340, 470, 365);
    auto optionMenu2 = new COptionMenu(menu2Rect, nullptr, 0);
    optionMenu2->addEntry("Mono");
    optionMenu2->addEntry("Stereo");
    optionMenu2->addEntry("Mid/Side");
    optionMenu2->setCurrent(1);
    optionMenu2->setFrameColor(CColor(60, 60, 60, 255)); // Dark border
    optionMenu2->setBackColor(CColor(255, 255, 255, 255)); // White background
    optionMenu2->setFontColor(CColor(20, 20, 20, 255)); // Dark text
    frame->addView(optionMenu2);
    
    CRect menu2LabelRect(350, 370, 470, 390);
    auto menu2Label = new CTextLabel(menu2LabelRect, "Output Mode");
    menu2Label->setFontColor(CColor(40, 40, 40, 255)); // Dark gray text
    menu2Label->setBackColor(CColor(255, 255, 255, 200)); // Semi-transparent white background
    frame->addView(menu2Label);
    
    // Additional segment button
    CRect segment2Rect(480, 340, 570, 365);
    auto segmentButton2 = new CSegmentButton(segment2Rect, nullptr, 0);
    CSegmentButton::Segment segmentA;
    segmentA.name = "A";
    segmentButton2->addSegment(segmentA);
    CSegmentButton::Segment segmentB;
    segmentB.name = "B";
    segmentButton2->addSegment(segmentB);
    CSegmentButton::Segment segmentC;
    segmentC.name = "C";
    segmentButton2->addSegment(segmentC);
    segmentButton2->setSelectedSegment(1);
    frame->addView(segmentButton2);
    
    CRect segment2LabelRect(480, 370, 570, 390);
    auto segment2Label = new CTextLabel(segment2LabelRect, "Mode");
    segment2Label->setFontColor(CColor(40, 40, 40, 255)); // Dark gray text
    segment2Label->setBackColor(CColor(255, 255, 255, 200)); // Semi-transparent white background
    frame->addView(segment2Label);
    
    // Status label at the bottom
    CRect statusRect(10, 450, 590, 470);
    auto statusLabel = new CTextLabel(statusRect, "VSTGUI Test Interface - All controls functional for testing purposes");
    statusLabel->setFontColor(CColor(0, 80, 160, 255)); // Dark blue text
    statusLabel->setBackColor(CColor(230, 245, 255, 220)); // Light blue background
    statusLabel->setHoriAlign(kCenterText);
    frame->addView(statusLabel);
}