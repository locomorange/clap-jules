#include "brisk_clap_integration.h"
#include "cross_platform_gui_support.h"
#include <iostream>
#include <cstring>
#include <mutex>

#include <brisk/core/internal/Initialization.hpp>
#include <brisk/core/Text.hpp>
#include <brisk/graphics/Geometry.hpp>
#include <brisk/graphics/Matrix.hpp>
#include <brisk/gui/Gui.hpp>
#include <brisk/gui/Event.hpp>
#include <brisk/widgets/Graphene.hpp>
#include <brisk/widgets/Text.hpp>
#include <brisk/widgets/Button.hpp>
#include <brisk/widgets/Slider.hpp>
#include <brisk/widgets/Layouts.hpp>
#include <fmt/core.h>

namespace brisk_clap {


// Native window adapter for CLAP
class ClapNativeWindow : public Brisk::NativeWindow {
public:
    ClapNativeWindow(const clap_window_t* parentWindow)
        : m_parentWindow(parentWindow) {
        // Initialize based on parent window API
        if (parentWindow && parentWindow->api) {
            m_handle = CrossPlatformGUISupport::extractNativeHandle(parentWindow);
            m_api = parentWindow->api;
            std::cout << "ClapNativeWindow: Created with API " << m_api << 
                         " on platform " << 
                         #ifdef _WIN32
                         "Windows" << 
                         #elif defined(__APPLE__)
                         "macOS" <<
                         #elif defined(__linux__)
                         "Linux" <<
                         #else
                         "Unknown" <<
                         #endif
                         std::endl;
        } else {
            std::cout << "ClapNativeWindow: Created with null parent window" << std::endl;
        }
    }

    Brisk::Size framebufferSize() const override {
        return m_framebufferSize;
    }

    void setFramebufferSize(const Brisk::Size& size) {
        m_framebufferSize = size;
        std::cout << "ClapNativeWindow: Set framebuffer size to " << size.width << "x" << size.height << std::endl;
    }

    Brisk::NativeWindowHandle getHandle() const override {
        if (!m_parentWindow) {
            return Brisk::NativeWindowHandle();
        }
        
        // Use cross-platform helper to extract native handle
        void* handle = CrossPlatformGUISupport::extractNativeHandle(m_parentWindow);
        
        // Create NativeWindowHandle with proper platform-specific constructor
        Brisk::NativeWindowHandle nativeHandle;
        nativeHandle.ptr = handle;
        return nativeHandle;
    }

    const char* getWindowAPI() const { return m_api; }

private:
    const clap_window_t* m_parentWindow = nullptr;
    void* m_handle = nullptr;
    const char* m_api = nullptr;
    Brisk::Size m_framebufferSize{ 400, 300 };
};

class BriskClapGUI::Impl {
public:
    std::unique_ptr<ClapNativeWindow> nativeWindow;
    Brisk::Rc<Brisk::RenderDevice> renderDevice;
    Brisk::Rc<Brisk::WindowRenderTarget> renderTarget;
    Brisk::Rc<Brisk::RenderEncoder> renderEncoder;
    Brisk::InputQueue inputQueue;
    std::unique_ptr<Brisk::WidgetTree> widgetTree;
    std::mutex renderMutex;

    // GUI state
    bool hasGUI = false;
    bool isVisible = false;
    double scaleFactor = 1.0;
    
    // Size constraints
    uint32_t minWidth = 300;
    uint32_t minHeight = 200;
    uint32_t maxWidth = 1200;
    uint32_t maxHeight = 800;
    
    // Parameters
    double cutoffFreq = 1000.0;
    double gain = 0.7;
    bool bypass = false;
    
    // Input processing  
    bool processMouseEvent(int x, int y, int button, bool pressed) {
        if (!widgetTree || !hasGUI) return false;
        
        // Scale coordinates based on DPI scaling
        float scaledX = static_cast<float>(x) / scaleFactor;
        float scaledY = static_cast<float>(y) / scaleFactor;
        
        // Create appropriate Brisk event and add to input queue
        if (pressed) {
            Brisk::EventMouseButtonPressed event;
            event.point = Brisk::PointF{scaledX, scaledY};
            event.button = static_cast<Brisk::MouseButton>(button);
            inputQueue.events.push_back(std::move(event));
        } else {
            Brisk::EventMouseButtonReleased event;
            event.point = Brisk::PointF{scaledX, scaledY};
            event.button = static_cast<Brisk::MouseButton>(button);
            inputQueue.events.push_back(std::move(event));
        }
        
        return true;
    }
    
    bool processKeyEvent(int keyCode, bool pressed) {
        if (!widgetTree || !hasGUI) return false;
        
        // Create appropriate Brisk keyboard event and add to input queue
        if (pressed) {
            Brisk::EventKeyPressed event;
            event.key = static_cast<Brisk::KeyCode>(keyCode);
            inputQueue.events.push_back(std::move(event));
        } else {
            Brisk::EventKeyReleased event;
            event.key = static_cast<Brisk::KeyCode>(keyCode);
            inputQueue.events.push_back(std::move(event));
        }
        
        return true;
    }
    
    void updateGUI() {
        if (!widgetTree || !hasGUI) return;
        
        std::lock_guard<std::mutex> lock(renderMutex);
        
        // Update widget tree 
        widgetTree->update();
        
        // Render if visible
        if (isVisible && renderTarget && renderEncoder) {
            Brisk::RenderPipeline pipeline(renderEncoder, renderTarget);
            Brisk::Canvas canvas(pipeline);
            
            // Apply scaling if needed
            if (scaleFactor != 1.0) {
                // Create identity matrix and apply scaling
                Brisk::Matrix matrix;  // Default constructor creates identity matrix
                canvas.transform(matrix.scale(scaleFactor, scaleFactor));
            }
            
            widgetTree->paint(canvas, Brisk::Palette::transparent, true);
            renderTarget->present();
        }
    }
};


BriskClapGUI::BriskClapGUI() : m_impl(std::make_unique<Impl>()) {
}

BriskClapGUI::~BriskClapGUI() = default;

bool BriskClapGUI::initialize() {
    if (m_initialized) {
        return true;
    }
    
    try {
        std::cout << "BriskClapGUI: Initializing with Brisk library" << std::endl;
        
        // Use global initializer to ensure Brisk is initialized only once
        BriskClapInitializer::instance().initialize();
        
        // Get render device (should be available after global initialization)
        auto deviceResult = Brisk::getRenderDevice();
        if (!deviceResult) {
            std::cerr << "BriskClapGUI: Failed to get render device" << std::endl;
            return false;
        }
        m_impl->renderDevice = deviceResult.value();
        m_impl->renderEncoder = m_impl->renderDevice->createEncoder();
        
        m_initialized = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "BriskClapGUI: Exception during initialization: " << e.what() << std::endl;
        return false;
    }
}

void BriskClapGUI::shutdown() {
    if (m_windowCreated) {
        destroyWindow();
    }
    
    // Note: Don't call BriskClapInitializer::shutdown() here as other instances might still be using Brisk
    // The global initializer will handle cleanup when the last instance is destroyed
    m_initialized = false;
}

bool BriskClapGUI::createWindow(const clap_window_t* parentWindow) {
    if (!m_initialized) {
        if (!initialize()) {
            return false;
        }
    }
    
    if (m_windowCreated) {
        return true;
    }

    try {
        std::cout << "BriskClapGUI: Creating window with parent API: " << (parentWindow ? parentWindow->api : "null") << std::endl;
        
        // Create native window adapter
        m_impl->nativeWindow = std::make_unique<ClapNativeWindow>(parentWindow);
        m_impl->nativeWindow->setFramebufferSize(Brisk::Size{ static_cast<int>(m_width), static_cast<int>(m_height) });
        
        // Create render target
        m_impl->renderTarget = m_impl->renderDevice->createWindowTarget(m_impl->nativeWindow.get(), Brisk::PixelType::U8);
        if (!m_impl->renderTarget) {
            std::cerr << "BriskClapGUI: Failed to create render target" << std::endl;
            return false;
        }
        
        // Create widget tree
        m_impl->widgetTree = std::make_unique<Brisk::WidgetTree>(&m_impl->inputQueue);
        
        // Create GUI layout
        createGUILayout();
        
        m_impl->hasGUI = true;
        m_windowCreated = true;
        
        std::cout << "BriskClapGUI: Window created successfully" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "BriskClapGUI: Exception creating window: " << e.what() << std::endl;
        return false;
    }
}

void BriskClapGUI::destroyWindow() {
    if (m_impl->hasGUI) {
        m_impl->widgetTree.reset();
        m_impl->renderTarget.reset();
        m_impl->nativeWindow.reset();
        m_impl->hasGUI = false;
        m_impl->isVisible = false;
    }
    m_windowCreated = false;
}

bool BriskClapGUI::setSize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    
    if (m_impl->nativeWindow) {
        m_impl->nativeWindow->setFramebufferSize(Brisk::Size{ static_cast<int>(width), static_cast<int>(height) });
    }
    
    return true;
}

void BriskClapGUI::getSize(uint32_t* width, uint32_t* height) const {
    if (width) *width = m_width;
    if (height) *height = m_height;
}

void BriskClapGUI::show() {

    if (m_impl->hasGUI && m_impl->widgetTree) {
        m_impl->isVisible = true;
        m_impl->updateGUI();
        std::cout << "BriskClapGUI: GUI shown" << std::endl;
    }

}

void BriskClapGUI::hide() {
    if (m_impl->hasGUI) {
        m_impl->isVisible = false;
        std::cout << "BriskClapGUI: GUI hidden" << std::endl;
    }
}

void BriskClapGUI::processEvents() {
    if (m_impl->hasGUI && m_impl->widgetTree) {
        // Process Brisk schedulers
        Brisk::mainScheduler->process();
        Brisk::uiScheduler->process();
        
        // Update GUI
        m_impl->updateGUI();
    }
}

bool BriskClapGUI::canResize() const {
    return true; // We support resizing
}

bool BriskClapGUI::adjustSize(uint32_t* width, uint32_t* height) const {
    if (!width || !height) return false;
    
    // Clamp to min/max sizes
    *width = std::max(m_impl->minWidth, std::min(*width, m_impl->maxWidth));
    *height = std::max(m_impl->minHeight, std::min(*height, m_impl->maxHeight));
    
    return true;
}

bool BriskClapGUI::setScale(double scale) {
    if (scale <= 0.0) return false;
    
    m_impl->scaleFactor = scale;
    
    // Update native window if created
    if (m_impl->nativeWindow) {
        Brisk::Size scaledSize{
            static_cast<int>(m_width * scale),
            static_cast<int>(m_height * scale)
        };
        m_impl->nativeWindow->setFramebufferSize(scaledSize);
    }
    
    std::cout << "BriskClapGUI: Scale factor set to " << scale << std::endl;
    return true;
}

bool BriskClapGUI::handleMouseEvent(int x, int y, int button, bool pressed) {
    if (!m_impl->hasGUI) return false;
    
    return m_impl->processMouseEvent(x, y, button, pressed);
}

bool BriskClapGUI::handleKeyEvent(int keyCode, bool pressed) {
    if (!m_impl->hasGUI) return false;
    
    return m_impl->processKeyEvent(keyCode, pressed);
}

bool BriskClapGUI::handleResizeEvent(uint32_t width, uint32_t height) {
    if (!setSize(width, height)) return false;
    
    // Update render target if necessary
    if (m_impl->renderTarget && m_impl->nativeWindow) {
        // Recreate render target with new size
        m_impl->renderTarget = m_impl->renderDevice->createWindowTarget(
            m_impl->nativeWindow.get(), 
            Brisk::PixelType::U8
        );
    }
    
    std::cout << "BriskClapGUI: Resized to " << width << "x" << height << std::endl;
    return true;
}

void BriskClapGUI::updateParameter(clap_id paramId, double value) {
    switch (paramId) {
        case 0: // Cutoff frequency
            m_impl->cutoffFreq = value;
            break;
        case 1: // Gain
            m_impl->gain = value;
            break;
        case 2: // Bypass
            m_impl->bypass = value > 0.5;
            break;
    }
    
    // Update GUI if available
    if (m_impl->hasGUI && m_impl->widgetTree) {
        m_impl->updateGUI();
    }
}


void BriskClapGUI::createGUILayout()
{
    using namespace Brisk;

    // Create a simple widget for the GUI root
    auto mainWidget = std::make_shared<Widget>();
    
    // Set the root widget - minimal implementation for now
    m_impl->widgetTree->setRoot(mainWidget);
    
    std::cout << "BriskClapGUI: Basic GUI layout created" << std::endl;
}

BriskClapInitializer& BriskClapInitializer::instance() {
    static BriskClapInitializer instance;
    return instance;
}

void BriskClapInitializer::initialize(int argc, char** argv) {
    if (m_initialized) {
        return;
    }

    try {
    // Initialize Brisk core for library/plugin usage
    // Note: Using initializeCommon/finalizeCommon avoids dependency on app-specific startup/shutdown
    Brisk::initializeCommon();
        
        // Try to register built-in fonts, but don't fail if resources are missing
        try {
            Brisk::registerBuiltinFonts();
        } catch (const std::exception& e) {
            std::cerr << "BriskClapInitializer: Warning - Could not register built-in fonts: " << e.what() << std::endl;
        }
        
        m_initialized = true;
        std::cout << "BriskClapInitializer: Brisk system initialized" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "BriskClapInitializer: Failed to initialize Brisk: " << e.what() << std::endl;
    }
}

void BriskClapInitializer::shutdown() {
    if (!m_initialized) {
        return;
    }

    try {
    // Finalize Brisk core for library/plugin usage
    Brisk::finalizeCommon();
    } catch (const std::exception& e) {
        fprintf(stderr, "Exception during Brisk shutdown: %s\n", e.what());
    }

    m_initialized = false;
}

} // namespace brisk_clap