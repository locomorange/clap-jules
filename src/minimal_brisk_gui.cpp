#include "minimal_brisk_gui.h"
#include <iostream>

#ifdef BRISK_FOUND
using namespace Brisk;
#endif

namespace clap_gui {

MinimalBriskGUI::MinimalBriskGUI() {
    std::cout << "MinimalBriskGUI: Constructor called" << std::endl;
}

MinimalBriskGUI::~MinimalBriskGUI() {
    cleanup_brisk_gui();
    std::cout << "MinimalBriskGUI: Destructor called" << std::endl;
}

bool MinimalBriskGUI::create(const clap_window_t* window, bool is_floating) {
    std::cout << "MinimalBriskGUI: create() called" << std::endl;
    
#ifdef BRISK_FOUND
    try {
        init_brisk_gui();
        m_initialized = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "MinimalBriskGUI: Failed to create GUI: " << e.what() << std::endl;
        return false;
    }
#else
    std::cout << "MinimalBriskGUI: Brisk not available, using placeholder" << std::endl;
    m_initialized = true;
    return true;
#endif
}

void MinimalBriskGUI::destroy() {
    std::cout << "MinimalBriskGUI: destroy() called" << std::endl;
    cleanup_brisk_gui();
    m_initialized = false;
}

bool MinimalBriskGUI::set_size(uint32_t width, uint32_t height) {
    std::cout << "MinimalBriskGUI: set_size(" << width << ", " << height << ")" << std::endl;
    
    // サイズの妥当性チェック
    if (width < 200 || height < 150) {
        std::cerr << "MinimalBriskGUI: Invalid size - minimum is 200x150" << std::endl;
        return false;
    }
    
    m_width = width;
    m_height = height;

#ifdef BRISK_FOUND
    if (m_window && m_initialized) {
        try {
            // Briskウィンドウのサイズを設定
            m_window->setSize({static_cast<int>(width), static_cast<int>(height)});
            return true;
        } catch (const std::exception& e) {
            std::cerr << "MinimalBriskGUI: Failed to set window size: " << e.what() << std::endl;
            return false;
        }
    }
#endif
    
    return true;
}

bool MinimalBriskGUI::get_size(uint32_t* width, uint32_t* height) {
    if (width) *width = m_width;
    if (height) *height = m_height;
    
    std::cout << "MinimalBriskGUI: get_size() -> " << m_width << "x" << m_height << std::endl;
    return true;
}

void MinimalBriskGUI::show() {
    std::cout << "MinimalBriskGUI: show() called" << std::endl;
#ifdef BRISK_FOUND
    if (m_window) {
        m_window->show();
    }
#endif
}

void MinimalBriskGUI::hide() {
    std::cout << "MinimalBriskGUI: hide() called" << std::endl;
#ifdef BRISK_FOUND
    if (m_window) {
        m_window->hide();
    }
#endif
}

void MinimalBriskGUI::set_parameter_value(clap_id param_id, double value) {
    std::cout << "MinimalBriskGUI: Parameter " << param_id << " set to " << value << std::endl;
    
    // パラメータIDに基づいて適切な値を更新
    switch (param_id) {
        case 0: // Volume parameter (仮のID)
            m_volume_value = value;
            break;
        case 1: // Bypass parameter (仮のID)
            m_bypass = value > 0.5;
            break;
        default:
            std::cout << "MinimalBriskGUI: Unknown parameter ID: " << param_id << std::endl;
            break;
    }
    
    // UIの更新をトリガー（必要に応じて）
#ifdef BRISK_FOUND
    if (m_window && m_initialized) {
        // Window::update() などの適切なメソッドがあるかチェックが必要
        // m_window->update();
    }
#endif
}

void MinimalBriskGUI::init_brisk_gui() {
#ifdef BRISK_FOUND
    std::cout << "MinimalBriskGUI: Initializing Brisk GUI..." << std::endl;
    
    try {
        // シンプルなBrisk GUI初期化
        // 実際のBrisk APIに合わせて後で実装
        std::cout << "MinimalBriskGUI: Brisk GUI initialized successfully (placeholder)" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "MinimalBriskGUI: Failed to initialize Brisk GUI: " << e.what() << std::endl;
        throw;
    }
#endif
}

void MinimalBriskGUI::cleanup_brisk_gui() {
#ifdef BRISK_FOUND
    if (m_window) {
        std::cout << "MinimalBriskGUI: Cleaning up Brisk GUI..." << std::endl;
        m_window.reset();
        m_root_widget.reset();
    }
#endif
}

#ifdef BRISK_FOUND
void MinimalBriskGUI::create_ui_layout() {
    // プレースホルダー実装
    // 実際のBrisk APIドキュメントに基づいて後で実装
    std::cout << "MinimalBriskGUI: Creating UI layout (placeholder)" << std::endl;
}

void MinimalBriskGUI::reset_parameters() {
    m_volume_value = 0.7;
    m_bypass = false;
    std::cout << "Parameters reset to defaults" << std::endl;
}
#endif

} // namespace clap_gui
