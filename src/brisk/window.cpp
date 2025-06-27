#include "brisk/gui/Window.hpp"
#include "native/native_window.hpp"

namespace brisk {

Window::Window(WindowStyle style, Size size, const std::string& title)
    : size_(size), visible_(false) {
    nativeWindow_ = std::make_unique<native::NativeWindow>(size.width, size.height, title);
}

Window::~Window() = default;

void Window::setVisible(bool visible) {
    visible_ = visible;
    if (nativeWindow_) {
        if (visible) {
            nativeWindow_->show();
            render(); // Render when shown
        } else {
            nativeWindow_->hide();
        }
    }
}

bool Window::visible() const {
    return visible_;
}

void Window::setSize(Size size) {
    size_ = size;
    if (nativeWindow_) {
        nativeWindow_->setSize(size.width, size.height);
        if (visible_) {
            render();
        }
    }
}

void Window::setPosition(Point pos) {
    if (nativeWindow_) {
        nativeWindow_->setPosition(pos.x, pos.y);
    }
}

void* Window::nativeHandle() const {
    return nativeWindow_ ? nativeWindow_->getNativeHandle() : nullptr;
}

void Window::setLayout(std::shared_ptr<Layout> layout) {
    layout_ = layout;
    if (visible_) {
        render();
    }
}

void Window::render() {
    if (!nativeWindow_ || !visible_) {
        return;
    }
    
    // Clear the window
    nativeWindow_->clear();
    
    // Render the layout if it exists
    if (layout_) {
        layout_->render(nativeWindow_.get(), 10, 10); // 10px margin
    }
    
    // Present the rendered content
    nativeWindow_->present();
}

}