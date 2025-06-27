#include "plugin/plugin_window.hpp"

using namespace brisk;

namespace plugin {

PluginWindow::PluginWindow(std::shared_ptr<PluginViewModel> viewModel)
    : viewModel_(viewModel) {
    createUI();
    bindViewModel();
    setupEventHandlers();
}

void PluginWindow::show() {
    if (window_) {
        window_->setVisible(true);
    }
}

void PluginWindow::hide() {
    if (window_) {
        window_->setVisible(false);
    }
}

bool PluginWindow::isVisible() const {
    return window_ ? window_->visible() : false;
}

void PluginWindow::setSize(int width, int height) {
    if (window_) {
        window_->setSize(Size{width, height});
    }
}

void PluginWindow::setPosition(int x, int y) {
    if (window_) {
        window_->setPosition(Point{x, y});
    }
}

void* PluginWindow::getNativeHandle() const {
    return window_ ? window_->nativeHandle() : nullptr;
}

void PluginWindow::createUI() {
    window_ = std::make_shared<Window>(
        WindowStyle::Dialog,
        Size{400, 300},
        "CLAP Plugin"
    );
    
    auto mainLayout = std::make_shared<VBoxLayout>();
    
    // Status label
    statusLabel_ = std::make_shared<Label>("Ready");
    statusLabel_->setTextAlign(TextAlign::Center);
    mainLayout->addWidget(statusLabel_);
    
    // Gain control
    auto gainLayout = std::make_shared<HBoxLayout>();
    gainLayout->addWidget(std::make_shared<Label>("Gain:"));
    gainSlider_ = std::make_shared<Slider>(0.0f, 2.0f, 1.0f);
    gainLayout->addWidget(gainSlider_);
    mainLayout->addWidget(gainLayout);
    
    // Frequency control
    auto freqLayout = std::make_shared<HBoxLayout>();
    freqLayout->addWidget(std::make_shared<Label>("Frequency:"));
    frequencySlider_ = std::make_shared<Slider>(20.0f, 20000.0f, 1000.0f);
    freqLayout->addWidget(frequencySlider_);
    mainLayout->addWidget(freqLayout);
    
    // Buttons
    auto buttonLayout = std::make_shared<HBoxLayout>();
    bypassButton_ = std::make_shared<Button>("Bypass");
    resetButton_ = std::make_shared<Button>("Reset");
    buttonLayout->addWidget(bypassButton_);
    buttonLayout->addWidget(resetButton_);
    mainLayout->addWidget(buttonLayout);
    
    window_->setLayout(mainLayout);
}

void PluginWindow::bindViewModel() {
    if (!viewModel_) return;
    
    // Bind properties to UI elements
    gainSlider_->setValue(viewModel_->gain.get());
    frequencySlider_->setValue(viewModel_->frequency.get());
    statusLabel_->setText(viewModel_->status.get());
    
    // Listen to property changes
    viewModel_->addPropertyChangedListener([this](const std::string& propertyName) {
        if (propertyName == "gain") {
            gainSlider_->setValue(viewModel_->gain.get());
        } else if (propertyName == "frequency") {
            frequencySlider_->setValue(viewModel_->frequency.get());
        } else if (propertyName == "status") {
            statusLabel_->setText(viewModel_->status.get());
        } else if (propertyName == "bypass") {
            bypassButton_->setText(viewModel_->bypass.get() ? "Bypass (ON)" : "Bypass (OFF)");
        }
    });
}

void PluginWindow::setupEventHandlers() {
    if (!viewModel_) return;
    
    // Slider event handlers
    gainSlider_->onValueChanged = [this](float value) {
        viewModel_->gain.set(value);
    };
    
    frequencySlider_->onValueChanged = [this](float value) {
        viewModel_->frequency.set(value);
    };
    
    // Button event handlers
    bypassButton_->onClick = [this]() {
        if (viewModel_->toggleBypassCommand) {
            viewModel_->toggleBypassCommand->execute();
        }
    };
    
    resetButton_->onClick = [this]() {
        if (viewModel_->resetCommand) {
            viewModel_->resetCommand->execute();
        }
    };
}

}