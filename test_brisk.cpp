#ifdef ENABLE_BRISK
#include <brisk/gui/GUIApplication.hpp>
#include <brisk/widgets/Widgets.hpp>
#include <brisk/gui/GUIWindow.hpp>
#include <iostream>

using namespace Brisk;

int main() {
    std::cout << "Testing Brisk GUI creation..." << std::endl;
    
    try {
        // Create a simple component
        auto component = rcnew Component{
            flexColumn,
            {
                rcnew Text("Hello Brisk!"),
                rcnew Button("Test Button"),
                rcnew Slider(0.0f, 1.0f, 0.5f),
            }
        };
        
        // Create window
        auto window = std::make_unique<GUIWindow>(component);
        window->setTitle("Brisk Test");
        window->setSize(Size{400.0f, 300.0f});
        window->show();
        
        std::cout << "Brisk GUI created successfully!" << std::endl;
        
        // Keep window open briefly
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
#else
#include <iostream>
int main() {
    std::cout << "Brisk not enabled" << std::endl;
    return 0;
}
#endif