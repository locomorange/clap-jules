#pragma once

#include <functional>
#include <memory>

namespace mvvm {

class CommandBase {
public:
    virtual ~CommandBase() = default;
    virtual void execute() = 0;
    virtual bool canExecute() const = 0;
};

class RelayCommand : public CommandBase {
public:
    using ExecuteCallback = std::function<void()>;
    using CanExecuteCallback = std::function<bool()>;
    
    explicit RelayCommand(ExecuteCallback execute)
        : execute_(execute), canExecute_([]() { return true; }) {}
    
    RelayCommand(ExecuteCallback execute, CanExecuteCallback canExecute)
        : execute_(execute), canExecute_(canExecute) {}
    
    void execute() override {
        if (canExecute()) {
            execute_();
        }
    }
    
    bool canExecute() const override {
        return canExecute_();
    }
    
private:
    ExecuteCallback execute_;
    CanExecuteCallback canExecute_;
};

template<typename T>
class ParameterCommand : public CommandBase {
public:
    using ExecuteCallback = std::function<void(const T&)>;
    using CanExecuteCallback = std::function<bool(const T&)>;
    
    ParameterCommand(ExecuteCallback execute, const T& parameter)
        : execute_(execute), parameter_(parameter), canExecute_([](const T&) { return true; }) {}
    
    ParameterCommand(ExecuteCallback execute, CanExecuteCallback canExecute, const T& parameter)
        : execute_(execute), parameter_(parameter), canExecute_(canExecute) {}
    
    void execute() override {
        if (canExecute()) {
            execute_(parameter_);
        }
    }
    
    bool canExecute() const override {
        return canExecute_(parameter_);
    }
    
    void setParameter(const T& parameter) {
        parameter_ = parameter;
    }
    
private:
    ExecuteCallback execute_;
    T parameter_;
    CanExecuteCallback canExecute_;
};

}