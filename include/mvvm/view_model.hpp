#pragma once

#include <memory>
#include <functional>
#include <vector>

namespace mvvm {

class ViewModelBase {
public:
    virtual ~ViewModelBase() = default;
    
    using PropertyChangedCallback = std::function<void(const std::string&)>;
    
    void addPropertyChangedListener(PropertyChangedCallback callback) {
        listeners_.push_back(callback);
    }
    
public:
    void notifyPropertyChanged(const std::string& propertyName) {
        for (const auto& listener : listeners_) {
            listener(propertyName);
        }
    }
    
private:
    std::vector<PropertyChangedCallback> listeners_;
};

template<typename T>
class Property {
public:
    Property(T value, ViewModelBase* viewModel, const std::string& propertyName)
        : value_(value), viewModel_(viewModel), propertyName_(propertyName) {}
    
    const T& get() const { return value_; }
    
    void set(const T& value) {
        if (value_ != value) {
            value_ = value;
            if (viewModel_) {
                viewModel_->notifyPropertyChanged(propertyName_);
            }
        }
    }
    
    operator const T&() const { return value_; }
    Property& operator=(const T& value) { set(value); return *this; }
    
private:
    T value_;
    ViewModelBase* viewModel_;
    std::string propertyName_;
};

}