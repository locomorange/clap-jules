#include "my_plugin_gui.h"
#include <vstgui/lib/controls/cknob.h>
#include <vstgui/lib/controls/cslider.h>
#include <vstgui/lib/controls/ctextlabel.h>
#include <vstgui/lib/controls/cbuttons.h>
#include <vstgui/lib/controls/cvumeter.h>
#include <vstgui/lib/cviewcontainer.h>
#include <vstgui/lib/vstguiinit.h>
#include <vstgui/vstgui.h>
#include <iostream>

MyPluginEditor::MyPluginEditor()
    : isCreated(false) 
    , isVisible(false)
    , currentWidth(400)
    , currentHeight(300)
    , parentWindowHandle(nullptr)
{
    // frame is initialized by the base class VSTGUIEditorInterface
}

MyPluginEditor::~MyPluginEditor() {
    destroy();
}

bool MyPluginEditor::isApiSupported(const char* api, bool isFloating) {
    // Support platform-specific APIs
#ifdef __linux__
    if (strcmp(api, CLAP_WINDOW_API_X11) == 0) {
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
        std::cout << "MyPlugin GUI: Already created, returning true" << std::endl;
        return true;
    }
    
    // Check if the API is supported for this platform
    if (!isApiSupported(api, isFloating)) {
        std::cout << "MyPlugin GUI: API not supported: " << api << std::endl;
        return false;
    }
    
    std::cout << "MyPlugin GUI: Creating frame..." << std::endl;
    
    try {
        // Create VSTGUI frame with proper size
        CRect rect(0, 0, currentWidth, currentHeight);
        std::cout << "MyPlugin GUI: Creating CFrame with rect " << rect.left << "," << rect.top << " " << rect.getWidth() << "x" << rect.getHeight() << std::endl;
        
        frame = new CFrame(rect, this);
        
        if (frame) {
            std::cout << "MyPlugin GUI: Frame created, adding controls..." << std::endl;
            createControls();
            isCreated = true;
            std::cout << "MyPlugin GUI: Created frame successfully" << std::endl;
            return true;
        } else {
            std::cout << "MyPlugin GUI: Failed to create CFrame" << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cout << "MyPlugin GUI: Exception creating GUI: " << e.what() << std::endl;
    }
    catch (...) {
        std::cout << "MyPlugin GUI: Unknown exception creating GUI" << std::endl;
    }
    
    return false;
}

void MyPluginEditor::destroy() {
    if (frame) {
        // Close the frame properly first
        if (frame->isAttached()) {
            frame->close();
        }
        frame->forget();
        frame = nullptr;
    }
    isCreated = false;
    isVisible = false;
    parentWindowHandle = nullptr;
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
        std::cout << "MyPlugin GUI: setParent failed - invalid state" << std::endl;
        return false;
    }
    
    try {
        std::cout << "MyPlugin GUI: setParent called with API: " << (window->api ? window->api : "null") << std::endl;
        
        // Handle platform-specific window embedding
#ifdef __linux__
        if (window->api && strcmp(window->api, CLAP_WINDOW_API_X11) == 0) {
            // Linux X11 embedding
            parentWindowHandle = (void*)(uintptr_t)window->x11;
            
            std::cout << "MyPlugin GUI: Embedding into X11 window ID: " << window->x11 << std::endl;
            
            // Validate that we have a reasonable window ID (not zero and not obviously invalid)
            if (window->x11 == 0) {
                std::cout << "MyPlugin GUI: Invalid X11 window ID (0)" << std::endl;
                return false;
            }
            
            // Open the frame with the X11 parent window
            try {
                if (frame->open(parentWindowHandle, PlatformType::kX11EmbedWindowID)) {
                    std::cout << "MyPlugin GUI: Successfully embedded into X11 window" << std::endl;
                    return true;
                } else {
                    std::cout << "MyPlugin GUI: Failed to open frame with X11 parent (window may not exist)" << std::endl;
                    return false;
                }
            } catch (const std::exception& e) {
                std::cout << "MyPlugin GUI: Exception during X11 frame open: " << e.what() << std::endl;
                return false;
            } catch (...) {
                std::cout << "MyPlugin GUI: Unknown exception during X11 frame open" << std::endl;
                return false;
            }
        }
#elif defined(__APPLE__)
        if (window->api && strcmp(window->api, CLAP_WINDOW_API_COCOA) == 0) {
            // macOS Cocoa embedding
            parentWindowHandle = window->cocoa;
            
            std::cout << "MyPlugin GUI: Embedding into Cocoa NSView" << std::endl;
            
            // Validate that we have a valid NSView pointer
            if (!window->cocoa) {
                std::cout << "MyPlugin GUI: Invalid Cocoa NSView (null)" << std::endl;
                return false;
            }
            
            // Open the frame with the Cocoa parent view
            try {
                if (frame->open(parentWindowHandle, PlatformType::kNSView)) {
                    std::cout << "MyPlugin GUI: Successfully embedded into Cocoa NSView" << std::endl;
                    return true;
                } else {
                    std::cout << "MyPlugin GUI: Failed to open frame with Cocoa parent" << std::endl;
                    return false;
                }
            } catch (const std::exception& e) {
                std::cout << "MyPlugin GUI: Exception during Cocoa frame open: " << e.what() << std::endl;
                return false;
            } catch (...) {
                std::cout << "MyPlugin GUI: Unknown exception during Cocoa frame open" << std::endl;
                return false;
            }
        }
#elif defined(_WIN32)
        if (window->api && strcmp(window->api, CLAP_WINDOW_API_WIN32) == 0) {
            // Windows Win32 embedding
            parentWindowHandle = (void*)window->win32;
            
            std::cout << "MyPlugin GUI: Embedding into Win32 HWND" << std::endl;
            
            // Validate that we have a valid HWND
            if (!window->win32) {
                std::cout << "MyPlugin GUI: Invalid Win32 HWND (null)" << std::endl;
                return false;
            }
            
            // Open the frame with the Win32 parent window
            try {
                if (frame->open(parentWindowHandle, PlatformType::kHWNDTopLevel)) {
                    std::cout << "MyPlugin GUI: Successfully embedded into Win32 HWND" << std::endl;
                    return true;
                } else {
                    std::cout << "MyPlugin GUI: Failed to open frame with Win32 parent" << std::endl;
                    return false;
                }
            } catch (const std::exception& e) {
                std::cout << "MyPlugin GUI: Exception during Win32 frame open: " << e.what() << std::endl;
                return false;
            } catch (...) {
                std::cout << "MyPlugin GUI: Unknown exception during Win32 frame open" << std::endl;
                return false;
            }
        }
#endif
        
        std::cout << "MyPlugin GUI: Unsupported window API: " << (window->api ? window->api : "null") << std::endl;
    }
    catch (const std::exception& e) {
        std::cout << "MyPlugin GUI: Error setting parent: " << e.what() << std::endl;
    }
    catch (...) {
        std::cout << "MyPlugin GUI: Unknown error setting parent" << std::endl;
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
        std::cout << "MyPlugin GUI: Cannot show - frame not created" << std::endl;
        return false;
    }
    
    try {
        // If frame is not yet opened (embedded), we can't show it
        if (!frame->isAttached()) {
            std::cout << "MyPlugin GUI: Cannot show - frame not embedded in parent window" << std::endl;
            return false;
        }
        
        frame->setVisible(true);
        isVisible = true;
        std::cout << "MyPlugin GUI: Shown successfully" << std::endl;
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
        std::cout << "MyPlugin GUI: Hidden successfully" << std::endl;
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
    
    // Set a light gray background for the frame
    frame->setBackgroundColor(CColor(240, 240, 240, 255));
    
    // Create a colorful header background
    CRect headerRect(0, 0, currentWidth, 40);
    auto headerView = new CViewContainer(headerRect);
    headerView->setBackgroundColor(CColor(70, 130, 180, 255)); // Steel blue
    frame->addView(headerView);
    
    // Create main title label with white text on blue background
    CRect titleRect(10, 8, currentWidth - 10, 32);
    auto titleLabel = new CTextLabel(titleRect, "My First CLAP Plugin - Test GUI");
    titleLabel->setFontColor(kWhiteCColor);
    titleLabel->setBackColor(kTransparentCColor);
    titleLabel->setHoriAlign(kCenterText);
    CFontRef font = new CFontDesc("Arial", 16, kBoldFace);
    titleLabel->setFont(font);
    headerView->addView(titleLabel);
    
    // Create colored background sections
    CRect leftPanelRect(10, 50, 190, 280);
    auto leftPanel = new CViewContainer(leftPanelRect);
    leftPanel->setBackgroundColor(CColor(255, 248, 220, 255)); // Cornsilk
    frame->addView(leftPanel);
    
    CRect rightPanelRect(200, 50, 390, 280);
    auto rightPanel = new CViewContainer(rightPanelRect);
    rightPanel->setBackgroundColor(CColor(240, 255, 255, 255)); // Azure
    frame->addView(rightPanel);
    
    // Add section labels
    CRect leftLabelRect(20, 55, 180, 75);
    auto leftLabel = new CTextLabel(leftLabelRect, "CONTROLS");
    leftLabel->setFontColor(CColor(139, 69, 19, 255)); // Saddle brown
    leftLabel->setBackColor(kTransparentCColor);
    leftLabel->setHoriAlign(kCenterText);
    CFontRef sectionFont = new CFontDesc("Arial", 12, kBoldFace);
    leftLabel->setFont(sectionFont);
    frame->addView(leftLabel);
    
    CRect rightLabelRect(210, 55, 380, 75);
    auto rightLabel = new CTextLabel(rightLabelRect, "VISUALIZATION");
    rightLabel->setFontColor(CColor(25, 25, 112, 255)); // Midnight blue
    rightLabel->setBackColor(kTransparentCColor);
    rightLabel->setHoriAlign(kCenterText);
    rightLabel->setFont(sectionFont);
    frame->addView(rightLabel);
    
    // Create an enhanced knob
    CRect knobRect(40, 90, 90, 140);
    auto knob = new CKnob(knobRect, nullptr, 0, nullptr, nullptr);
    knob->setDefaultValue(0.7f);
    knob->setValue(0.7f);
    leftPanel->addView(knob);
    
    // Enhanced knob label
    CRect knobLabelRect(30, 145, 100, 165);
    auto knobLabel = new CTextLabel(knobLabelRect, "VOLUME");
    knobLabel->setFontColor(CColor(139, 69, 19, 255));
    knobLabel->setBackColor(kTransparentCColor);
    knobLabel->setHoriAlign(kCenterText);
    knobLabel->setFont(sectionFont);
    leftPanel->addView(knobLabel);
    
    // Create an enhanced horizontal slider
    CRect sliderRect(30, 180, 160, 200);
    auto slider = new CHorizontalSlider(sliderRect, nullptr, 0, 0, 100, nullptr, nullptr);
    slider->setValue(65.0f);
    slider->setBackColor(CColor(255, 255, 255, 255));
    leftPanel->addView(slider);
    
    // Enhanced slider label
    CRect sliderLabelRect(30, 205, 160, 225);
    auto sliderLabel = new CTextLabel(sliderLabelRect, "FILTER CUTOFF");
    sliderLabel->setFontColor(CColor(139, 69, 19, 255));
    sliderLabel->setBackColor(kTransparentCColor);
    sliderLabel->setHoriAlign(kCenterText);
    sliderLabel->setFont(sectionFont);
    leftPanel->addView(sliderLabel);
    
    // Add a colorful button
    CRect buttonRect(50, 235, 120, 255);
    auto button = new CTextButton(buttonRect, nullptr, 0, "TEST");
    button->setGradient(CGradient::create(0, 1, CColor(255, 215, 0, 255), CColor(255, 140, 0, 255))); // Gold gradient
    button->setTextColor(CColor(139, 69, 19, 255));
    button->setFont(sectionFont);
    leftPanel->addView(button);
    
    // Create visual elements in the right panel
    // Colored rectangles for testing
    CRect rect1(220, 90, 270, 120);
    auto colorView1 = new CViewContainer(rect1);
    colorView1->setBackgroundColor(CColor(255, 99, 71, 255)); // Tomato
    rightPanel->addView(colorView1);
    
    CRect rect2(280, 90, 330, 120);
    auto colorView2 = new CViewContainer(rect2);
    colorView2->setBackgroundColor(CColor(60, 179, 113, 255)); // Medium sea green
    rightPanel->addView(colorView2);
    
    CRect rect3(340, 90, 370, 120);
    auto colorView3 = new CViewContainer(rect3);
    colorView3->setBackgroundColor(CColor(147, 112, 219, 255)); // Medium purple
    rightPanel->addView(colorView3);
    
    // Add a VU meter for visual feedback
    CRect vuRect(230, 140, 260, 220);
    auto vuMeter = new CVuMeter(vuRect, nullptr, nullptr, 0, CVuMeter::kVertical);
    vuMeter->setValue(0.6f);
    vuMeter->setDecreaseStepValue(0.05f);
    rightPanel->addView(vuMeter);
    
    // Add labels for the visual elements
    CRect colorLabelRect(280, 125, 330, 140);
    auto colorLabel = new CTextLabel(colorLabelRect, "Colors");
    colorLabel->setFontColor(CColor(25, 25, 112, 255));
    colorLabel->setBackColor(kTransparentCColor);
    colorLabel->setHoriAlign(kCenterText);
    colorLabel->setFont(sectionFont);
    rightPanel->addView(colorLabel);
    
    CRect vuLabelRect(225, 225, 265, 240);
    auto vuLabel = new CTextLabel(vuLabelRect, "Level");
    vuLabel->setFontColor(CColor(25, 25, 112, 255));
    vuLabel->setBackColor(kTransparentCColor);
    vuLabel->setHoriAlign(kCenterText);
    vuLabel->setFont(sectionFont);
    rightPanel->addView(vuLabel);
    
    // Add some status text
    CRect statusRect(280, 160, 370, 200);
    auto statusLabel = new CTextLabel(statusRect, "GUI Test\nStatus: OK\nVersion: 1.0");
    statusLabel->setFontColor(CColor(25, 25, 112, 255));
    statusLabel->setBackColor(kTransparentCColor);
    statusLabel->setHoriAlign(kLeftText);
    rightPanel->addView(statusLabel);
    
    std::cout << "MyPlugin GUI: Created enhanced test controls with colors and visual elements" << std::endl;
}