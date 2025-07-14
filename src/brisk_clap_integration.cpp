#include "brisk_clap_integration.h"
#include <iostream>
#include <cstring>
#include <mutex>

#include <brisk/core/internal/Initialization.hpp>
#include <brisk/core/Text.hpp>
#include <brisk/graphics/Geometry.hpp>
#include <brisk/gui/Gui.hpp>
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
        if (parentWindow) {
            m_handle = parentWindow->ptr;
            m_api = parentWindow->api;
            std::cout << "ClapNativeWindow: Created with API " << m_api << std::endl;
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
        Brisk::NativeWindowHandle handle;
        handle.ptr = m_handle;
        return handle;
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
    bool briskInitialized = false;
    std::mutex renderMutex;

    // GUI state
    bool hasGUI = false;
    bool isVisible = false;
    
    // Parameters
    double cutoffFreq = 1000.0;
    double gain = 0.7;
    bool bypass = false;
    
    // GUI controls
    Brisk::Rc<Brisk::Slider> cutoffSlider;
    Brisk::Rc<Brisk::Slider> gainSlider;
    Brisk::Rc<Brisk::Button> bypassButton;
    Brisk::Rc<Brisk::Text> cutoffLabel;
    Brisk::Rc<Brisk::Text> gainLabel;
    
    void updateGUI() {
        if (!widgetTree || !hasGUI) return;
        
        std::lock_guard<std::mutex> lock(renderMutex);
        
        // Update widget tree
        widgetTree->update();
        
        // Render if visible
        if (isVisible && renderTarget && renderEncoder) {
            Brisk::RenderPipeline pipeline(renderEncoder, renderTarget);
            Brisk::Canvas canvas(pipeline);
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
        
        // Initialize Brisk if not already done
        if (!m_impl->briskInitialized) {
            // Initialize Brisk system
            Brisk::startup(0, nullptr);
            Brisk::registerBuiltinFonts();
            
            auto deviceResult = Brisk::getRenderDevice();
            if (!deviceResult) {
                std::cerr << "BriskClapGUI: Failed to get render device" << std::endl;
                return false;
            }
            m_impl->renderDevice = deviceResult.value();
            m_impl->renderEncoder = m_impl->renderDevice->createEncoder();
            m_impl->briskInitialized = true;
        }
        
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
    
    if (m_impl->briskInitialized) {
        try {
            Brisk::shutdown();
        } catch (const std::exception& e) {
            std::cerr << "BriskClapGUI: Exception during Brisk shutdown: " << e.what() << std::endl;
        }
        m_impl->briskInitialized = false;
    }
    
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

    // Create a minimal widget for the GUI root
    auto mainWidget = std::make_shared<Widget>();
    
    // Just set the root widget - minimal implementation
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
        Brisk::startup(argc, argv);
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
        Brisk::shutdown();
    } catch (const std::exception& e) {
        fprintf(stderr, "Exception during Brisk shutdown: %s\n", e.what());
    }

    m_initialized = false;
}

} // namespace brisk_clap