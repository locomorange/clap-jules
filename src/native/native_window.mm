#include "native/native_window.hpp"
#include <iostream>

#ifdef __APPLE__
#import <Cocoa/Cocoa.h>

namespace native {

NativeWindow::NativeWindow(int width, int height, const std::string& title)
    : width_(width), height_(height), title_(title), visible_(false) {
    window_ = nullptr;
    view_ = nullptr;
    createWindow();
}

NativeWindow::~NativeWindow() {
    destroyWindow();
}

void NativeWindow::createWindow() {
    @autoreleasepool {
        // Create NSWindow
        NSRect frame = NSMakeRect(100, 100, width_, height_);
        NSWindow* nsWindow = [[NSWindow alloc] 
            initWithContentRect:frame
            styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable)
            backing:NSBackingStoreBuffered
            defer:NO];
        
        [nsWindow setTitle:[NSString stringWithUTF8String:title_.c_str()]];
        [nsWindow setBackgroundColor:[NSColor whiteColor]];
        
        // Create content view
        NSView* contentView = [[NSView alloc] initWithFrame:frame];
        [nsWindow setContentView:contentView];
        
        // Store as opaque pointers
        window_ = (__bridge_retained void*)nsWindow;
        view_ = (__bridge_retained void*)contentView;
    }
}

void NativeWindow::destroyWindow() {
    if (window_) {
        NSWindow* nsWindow = (__bridge_transfer NSWindow*)window_;
        nsWindow = nil;
        window_ = nullptr;
    }
    if (view_) {
        NSView* nsView = (__bridge_transfer NSView*)view_;
        nsView = nil;
        view_ = nullptr;
    }
}

void NativeWindow::show() {
    if (window_) {
        NSWindow* nsWindow = (__bridge NSWindow*)window_;
        [nsWindow makeKeyAndOrderFront:nil];
        visible_ = true;
    }
}

void NativeWindow::hide() {
    if (window_) {
        NSWindow* nsWindow = (__bridge NSWindow*)window_;
        [nsWindow orderOut:nil];
        visible_ = false;
    }
}

bool NativeWindow::isVisible() const {
    return visible_;
}

void NativeWindow::setSize(int width, int height) {
    width_ = width;
    height_ = height;
    
    if (window_) {
        NSWindow* nsWindow = (__bridge NSWindow*)window_;
        NSRect frame = [nsWindow frame];
        frame.size.width = width;
        frame.size.height = height;
        [nsWindow setFrame:frame display:YES];
    }
}

void NativeWindow::setPosition(int x, int y) {
    if (window_) {
        NSWindow* nsWindow = (__bridge NSWindow*)window_;
        NSRect frame = [nsWindow frame];
        frame.origin.x = x;
        frame.origin.y = y;
        [nsWindow setFrame:frame display:YES];
    }
}

void* NativeWindow::getNativeHandle() const {
    return window_;
}

void NativeWindow::setParent(void* parent) {
    if (window_ && parent) {
        NSWindow* nsWindow = (__bridge NSWindow*)window_;
        NSWindow* parentWindow = (__bridge NSWindow*)parent;
        [parentWindow addChildWindow:nsWindow ordered:NSWindowAbove];
    }
}

void NativeWindow::drawText(const std::string& text, int x, int y) {
    if (!view_) return;
    
    @autoreleasepool {
        NSView* nsView = (__bridge NSView*)view_;
        NSString* nsText = [NSString stringWithUTF8String:text.c_str()];
        
        // Simple text drawing using NSString drawing
        NSDictionary* attributes = @{
            NSFontAttributeName: [NSFont systemFontOfSize:12.0],
            NSForegroundColorAttributeName: [NSColor blackColor]
        };
        
        NSPoint point = NSMakePoint(x, [nsView bounds].size.height - y); // Flip Y coordinate
        [nsText drawAtPoint:point withAttributes:attributes];
    }
}

void NativeWindow::drawRect(int x, int y, int width, int height) {
    if (!view_) return;
    
    @autoreleasepool {
        NSView* nsView = (__bridge NSView*)view_;
        NSRect rect = NSMakeRect(x, [nsView bounds].size.height - y - height, width, height); // Flip Y
        
        [[NSColor blackColor] setStroke];
        NSBezierPath* path = [NSBezierPath bezierPathWithRect:rect];
        [path setLineWidth:1.0];
        [path stroke];
    }
}

void NativeWindow::clear() {
    if (!view_) return;
    
    @autoreleasepool {
        NSView* nsView = (__bridge NSView*)view_;
        [nsView setNeedsDisplay:YES];
    }
}

void NativeWindow::present() {
    if (window_) {
        NSWindow* nsWindow = (__bridge NSWindow*)window_;
        [nsWindow display];
    }
}

}

#endif // __APPLE__