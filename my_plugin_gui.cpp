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
    , currentWidth(900)  // より大きなウィンドウサイズ (Larger window size)
    , currentHeight(600) // より大きなウィンドウサイズ (Larger window size)
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
    
    // プロフェッショナルなsoothe2風のレイアウトを作成 (Create professional soothe2-style layout)
    // 背景色をダークテーマに設定 (Set background to dark theme)
    frame->setBackgroundColor(CColor(28, 30, 34, 255)); // ダークグレー背景 (Dark gray background)
    
    // === ヘッダータイトル (Header title) ===
    CRect titleRect(20, 15, 880, 45);
    auto titleLabel = new CTextLabel(titleRect, "CLAP Audio Plugin - Professional EQ Interface");
    titleLabel->setFontColor(CColor(220, 220, 220, 255)); // 明るいグレーテキスト (Light gray text)
    titleLabel->setBackColor(CColor(0, 0, 0, 0)); // 透明背景 (Transparent background)
    titleLabel->setHoriAlign(kCenterText);
    CFontRef font = kSystemFont;
    titleLabel->setFont(font);
    frame->addView(titleLabel);
    
    // === 左側制御パネル (Left control panel) ===
    
    // メインDepthノブ（大きなノブ） (Main Depth knob - large knob)
    CRect depthKnobRect(40, 70, 120, 150);
    auto depthKnob = new CKnob(depthKnobRect, nullptr, PARAM_DEPTH, nullptr, nullptr);
    depthKnob->setDefaultValue(0.5f);
    depthKnob->setValue(0.5f);
    depthKnob->setColorShadowHandle(CColor(50, 50, 50, 255));
    depthKnob->setColorHandle(CColor(100, 180, 255, 255)); // 青色ハンドル (Blue handle)
    frame->addView(depthKnob);
    
    CRect depthLabelRect(25, 155, 135, 175);
    auto depthLabel = new CTextLabel(depthLabelRect, "DEPTH");
    depthLabel->setFontColor(CColor(200, 200, 200, 255));
    depthLabel->setBackColor(CColor(0, 0, 0, 0));
    depthLabel->setHoriAlign(kCenterText);
    frame->addView(depthLabel);
    
    // Sharpnessノブ (Sharpness knob)
    CRect sharpnessKnobRect(40, 200, 100, 260);
    auto sharpnessKnob = new CKnob(sharpnessKnobRect, nullptr, PARAM_SHARPNESS, nullptr, nullptr);
    sharpnessKnob->setDefaultValue(0.7f);
    sharpnessKnob->setValue(0.7f);
    sharpnessKnob->setColorShadowHandle(CColor(50, 50, 50, 255));
    sharpnessKnob->setColorHandle(CColor(255, 150, 80, 255)); // オレンジハンドル (Orange handle)
    frame->addView(sharpnessKnob);
    
    CRect sharpnessLabelRect(25, 265, 115, 285);
    auto sharpnessLabel = new CTextLabel(sharpnessLabelRect, "SHARPNESS");
    sharpnessLabel->setFontColor(CColor(200, 200, 200, 255));
    sharpnessLabel->setBackColor(CColor(0, 0, 0, 0));
    sharpnessLabel->setHoriAlign(kCenterText);
    frame->addView(sharpnessLabel);
    
    // Selectivityノブ (Selectivity knob)
    CRect selectivityKnobRect(140, 200, 200, 260);
    auto selectivityKnob = new CKnob(selectivityKnobRect, nullptr, PARAM_SELECTIVITY, nullptr, nullptr);
    selectivityKnob->setDefaultValue(0.6f);
    selectivityKnob->setValue(0.6f);
    selectivityKnob->setColorShadowHandle(CColor(50, 50, 50, 255));
    selectivityKnob->setColorHandle(CColor(150, 255, 150, 255)); // 緑ハンドル (Green handle)
    frame->addView(selectivityKnob);
    
    CRect selectivityLabelRect(125, 265, 215, 285);
    auto selectivityLabel = new CTextLabel(selectivityLabelRect, "SELECTIVITY");
    selectivityLabel->setFontColor(CColor(200, 200, 200, 255));
    selectivityLabel->setBackColor(CColor(0, 0, 0, 0));
    selectivityLabel->setHoriAlign(kCenterText);
    frame->addView(selectivityLabel);
    
    // Attack/Releaseノブ (Attack/Release knobs)
    CRect attackKnobRect(40, 310, 90, 360);
    auto attackKnob = new CKnob(attackKnobRect, nullptr, PARAM_ATTACK, nullptr, nullptr);
    attackKnob->setDefaultValue(0.01f);
    attackKnob->setValue(0.01f);
    attackKnob->setColorShadowHandle(CColor(50, 50, 50, 255));
    attackKnob->setColorHandle(CColor(255, 120, 120, 255)); // 赤ハンドル (Red handle)
    frame->addView(attackKnob);
    
    CRect attackLabelRect(25, 365, 105, 385);
    auto attackLabel = new CTextLabel(attackLabelRect, "ATTACK");
    attackLabel->setFontColor(CColor(200, 200, 200, 255));
    attackLabel->setBackColor(CColor(0, 0, 0, 0));
    attackLabel->setHoriAlign(kCenterText);
    frame->addView(attackLabel);
    
    CRect releaseKnobRect(110, 310, 160, 360);
    auto releaseKnob = new CKnob(releaseKnobRect, nullptr, PARAM_RELEASE, nullptr, nullptr);
    releaseKnob->setDefaultValue(0.1f);
    releaseKnob->setValue(0.1f);
    releaseKnob->setColorShadowHandle(CColor(50, 50, 50, 255));
    releaseKnob->setColorHandle(CColor(255, 255, 120, 255)); // 黄ハンドル (Yellow handle)
    frame->addView(releaseKnob);
    
    CRect releaseLabelRect(95, 365, 175, 385);
    auto releaseLabel = new CTextLabel(releaseLabelRect, "RELEASE");
    releaseLabel->setFontColor(CColor(200, 200, 200, 255));
    releaseLabel->setBackColor(CColor(0, 0, 0, 0));
    releaseLabel->setHoriAlign(kCenterText);
    frame->addView(releaseLabel);
    
    // === モード選択とスイッチ (Mode selection and switches) ===
    
    // Modeラジオボタン (Mode radio buttons)
    CRect modeSegmentRect(40, 410, 200, 440);
    auto modeSegment = new CSegmentButton(modeSegmentRect, nullptr, PARAM_MODE);
    CSegmentButton::Segment mode1;
    mode1.name = "Soft";
    modeSegment->addSegment(mode1);
    CSegmentButton::Segment mode2;
    mode2.name = "Medium";
    modeSegment->addSegment(mode2);
    CSegmentButton::Segment mode3;
    mode3.name = "Hard";
    modeSegment->addSegment(mode3);
    modeSegment->setSelectedSegment(0);
    frame->addView(modeSegment);
    
    CRect modeLabelRect(40, 445, 200, 465);
    auto modeLabel = new CTextLabel(modeLabelRect, "MODE");
    modeLabel->setFontColor(CColor(200, 200, 200, 255));
    modeLabel->setBackColor(CColor(0, 0, 0, 0));
    frame->addView(modeLabel);
    
    // Stereo Linkトグル (Stereo Link toggle)
    CRect stereoToggleRect(40, 480, 120, 505);
    auto stereoToggle = new COnOffButton(stereoToggleRect, nullptr, PARAM_STEREO_LINK, nullptr, 0);
    stereoToggle->setValue(1.0f);
    frame->addView(stereoToggle);
    
    CRect stereoLabelRect(40, 510, 120, 530);
    auto stereoLabel = new CTextLabel(stereoLabelRect, "STEREO LINK");
    stereoLabel->setFontColor(CColor(200, 200, 200, 255));
    stereoLabel->setBackColor(CColor(0, 0, 0, 0));
    frame->addView(stereoLabel);
    
    // Qualityスイッチ (Quality switch)
    CRect qualityToggleRect(140, 480, 200, 505);
    auto qualityToggle = new COnOffButton(qualityToggleRect, nullptr, PARAM_QUALITY, nullptr, 0);
    qualityToggle->setValue(1.0f);
    frame->addView(qualityToggle);
    
    CRect qualityLabelRect(140, 510, 200, 530);
    auto qualityLabel = new CTextLabel(qualityLabelRect, "QUALITY");
    qualityLabel->setFontColor(CColor(200, 200, 200, 255));
    qualityLabel->setBackColor(CColor(0, 0, 0, 0));
    frame->addView(qualityLabel);
    
    // Balanceスライダー (Balance slider)
    CRect balanceSliderRect(40, 550, 200, 570);
    auto balanceSlider = new CHorizontalSlider(balanceSliderRect, nullptr, PARAM_BALANCE, -1.0, 1.0, nullptr, nullptr);
    balanceSlider->setValue(0.0f);
    balanceSlider->setFrameColor(CColor(100, 100, 100, 255));
    balanceSlider->setBackColor(CColor(50, 50, 50, 255));
    balanceSlider->setValueColor(CColor(100, 180, 255, 255));
    frame->addView(balanceSlider);
    
    CRect balanceLabelRect(40, 575, 200, 595);
    auto balanceLabel = new CTextLabel(balanceLabelRect, "BALANCE");
    balanceLabel->setFontColor(CColor(200, 200, 200, 255));
    balanceLabel->setBackColor(CColor(0, 0, 0, 0));
    frame->addView(balanceLabel);
    
    // === 右側EQカーブエリア (Right side EQ curve area) ===
    
    // EQカーブビューを作成 (Create EQ curve view)
    CRect eqCurveRect(280, 70, 860, 420);
    // TODO: カスタムEQCurveViewクラスを使用 (TODO: Use custom EQCurveView class)
    // auto eqCurveView = new EQCurveView(eqCurveRect);
    // frame->addView(eqCurveView);
    
    // 現在は簡易的な表示エリアを作成 (Create simplified display area for now)
    auto eqBackground = new CView(eqCurveRect);
    // CViewではsetBackgroundColorではなくdraw()をオーバーライドして背景を描画 (For CView, override draw() instead of setBackgroundColor)
    frame->addView(eqBackground);
    
    CRect eqTitleRect(280, 45, 860, 70);
    auto eqTitle = new CTextLabel(eqTitleRect, "EQ CURVE - Auto Dynamic EQ (Double-click to add points, Right-click to remove)");
    eqTitle->setFontColor(CColor(180, 180, 180, 255));
    eqTitle->setBackColor(CColor(0, 0, 0, 0));
    eqTitle->setHoriAlign(kCenterText);
    frame->addView(eqTitle);
    
    // === 下部制御エリア (Bottom control area) ===
    
    // Output Gainノブ (Output Gain knob)
    CRect outputGainRect(280, 450, 340, 510);
    auto outputGainKnob = new CKnob(outputGainRect, nullptr, PARAM_OUTPUT_GAIN, nullptr, nullptr);
    outputGainKnob->setDefaultValue(1.0f);
    outputGainKnob->setValue(1.0f);
    outputGainKnob->setColorShadowHandle(CColor(50, 50, 50, 255));
    outputGainKnob->setColorHandle(CColor(255, 200, 100, 255)); // オレンジハンドル (Orange handle)
    frame->addView(outputGainKnob);
    
    CRect outputGainLabelRect(265, 515, 355, 535);
    auto outputGainLabel = new CTextLabel(outputGainLabelRect, "OUTPUT GAIN");
    outputGainLabel->setFontColor(CColor(200, 200, 200, 255));
    outputGainLabel->setBackColor(CColor(0, 0, 0, 0));
    outputGainLabel->setHoriAlign(kCenterText);
    frame->addView(outputGainLabel);
    
    // Bypassボタン (Bypass button)
    CRect bypassButtonRect(380, 465, 460, 495);
    auto bypassButton = new CTextButton(bypassButtonRect, nullptr, PARAM_BYPASS, "BYPASS");
    bypassButton->setGradient(CGradient::create(0.0, 1.0, CColor(70, 70, 70, 255), CColor(50, 50, 50, 255)));
    bypassButton->setFrameColor(CColor(100, 100, 100, 255));
    bypassButton->setTextColor(CColor(220, 220, 220, 255));
    frame->addView(bypassButton);
    
    // EQ制御ボタン (EQ control buttons)
    CRect resetButtonRect(500, 450, 580, 480);
    auto resetButton = new CTextButton(resetButtonRect, nullptr, -1, "RESET");
    resetButton->setGradient(CGradient::create(0.0, 1.0, CColor(70, 70, 70, 255), CColor(50, 50, 50, 255)));
    resetButton->setFrameColor(CColor(100, 100, 100, 255));
    resetButton->setTextColor(CColor(220, 220, 220, 255));
    frame->addView(resetButton);
    
    CRect undoButtonRect(590, 450, 650, 480);
    auto undoButton = new CTextButton(undoButtonRect, nullptr, -1, "UNDO");
    undoButton->setGradient(CGradient::create(0.0, 1.0, CColor(70, 70, 70, 255), CColor(50, 50, 50, 255)));
    undoButton->setFrameColor(CColor(100, 100, 100, 255));
    undoButton->setTextColor(CColor(220, 220, 220, 255));
    frame->addView(undoButton);
    
    CRect redoButtonRect(660, 450, 720, 480);
    auto redoButton = new CTextButton(redoButtonRect, nullptr, -1, "REDO");
    redoButton->setGradient(CGradient::create(0.0, 1.0, CColor(70, 70, 70, 255), CColor(50, 50, 50, 255)));
    redoButton->setFrameColor(CColor(100, 100, 100, 255));
    redoButton->setTextColor(CColor(220, 220, 220, 255));
    frame->addView(redoButton);
    
    // ステータス表示 (Status display)
    CRect statusRect(500, 485, 720, 515);
    auto statusLabel = new CTextLabel(statusRect, "Professional Audio Plugin Ready");
    statusLabel->setFontColor(CColor(120, 180, 120, 255)); // 緑色のステータス (Green status)
    statusLabel->setBackColor(CColor(0, 0, 0, 0));
    statusLabel->setHoriAlign(kCenterText);
    frame->addView(statusLabel);
    
    // 版権表示 (Copyright notice)
    CRect copyrightRect(20, 550, 250, 580);
    auto copyrightLabel = new CTextLabel(copyrightRect, "© 2025 CLAP Audio Plugin");
    copyrightLabel->setFontColor(CColor(100, 100, 100, 255));
    copyrightLabel->setBackColor(CColor(0, 0, 0, 0));
    frame->addView(copyrightLabel);
}