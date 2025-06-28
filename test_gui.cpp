#include "simple_gui.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Testing SimpleGUI..." << std::endl;
    
    SimpleGUI gui;
    
    if (!gui.create()) {
        std::cerr << "Failed to create GUI" << std::endl;
        return 1;
    }
    
    gui.set_title("Test CLAP GUI");
    
    if (!gui.show()) {
        std::cerr << "Failed to show GUI" << std::endl;
        return 1;
    }
    
    std::cout << "GUI created and shown. Waiting 5 seconds..." << std::endl;
    
    // Keep window open for 5 seconds
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    gui.hide();
    gui.destroy();
    
    std::cout << "Test completed successfully!" << std::endl;
    return 0;
}