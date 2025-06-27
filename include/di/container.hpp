#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <functional>

namespace dependency_injection {

// Simplified DI container without Boost.DI for now
class Container {
public:
    template<typename T>
    static std::shared_ptr<T> resolve() {
        auto it = factories_.find(std::type_index(typeid(T)));
        if (it != factories_.end()) {
            return std::static_pointer_cast<T>(it->second());
        }
        return std::make_shared<T>();
    }
    
    template<typename TInterface, typename TImplementation>
    static void bind() {
        factories_[std::type_index(typeid(TInterface))] = []() -> std::shared_ptr<void> {
            return std::make_shared<TImplementation>();
        };
    }
    
    template<typename T>
    static void bindSingleton(std::shared_ptr<T> instance) {
        factories_[std::type_index(typeid(T))] = [instance]() -> std::shared_ptr<void> {
            return instance;
        };
    }
    
    static void configure() {
        // Simple configuration
    }
    
private:
    static inline std::unordered_map<std::type_index, std::function<std::shared_ptr<void>()>> factories_;
};

template<typename TInterface>
class ServiceLocator {
public:
    static std::shared_ptr<TInterface> get() {
        return Container::resolve<TInterface>();
    }
};

}