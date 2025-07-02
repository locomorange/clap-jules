#pragma once

// Placeholder brisk header - replace with actual brisk integration
// when prebuilt binaries are available

#include <functional>
#include <algorithm>

namespace brisk {

class Application {
public:
    static void Initialize() {
        // TODO: Implement brisk initialization
    }
    
    static void Shutdown() {
        // TODO: Implement brisk shutdown
    }
};

class Window {
public:
    Window(void* parent_handle) : parent_handle_(parent_handle) {
        // TODO: Create brisk window
    }
    
    void SetVisible(bool visible) {
        visible_ = visible;
        // TODO: Implement window visibility
    }
    
    void* GetHandle() const {
        return parent_handle_;
    }
    
    void Render() {
        // TODO: Implement brisk rendering
    }
    
private:
    void* parent_handle_;
    bool visible_ = false;
};

class Knob {
public:
    Knob(double min_val, double max_val, double initial_val)
        : min_value_(min_val), max_value_(max_val), current_value_(initial_val) {
        // TODO: Create brisk knob control
    }
    
    void SetValue(double value) {
        current_value_ = std::max(min_value_, std::min(max_value_, value));
        // TODO: Update UI
    }
    
    double GetValue() const {
        return current_value_;
    }
    
    void SetCallback(std::function<void(double)> callback) {
        callback_ = callback;
    }
    
private:
    double min_value_;
    double max_value_;
    double current_value_;
    std::function<void(double)> callback_;
};

} // namespace brisk
