#include "renderer.h"
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <stdio.h>
#include <string.h>

bool renderer_init(simple_renderer_t* renderer, GLFWwindow* window, uint32_t width, uint32_t height) {
    if (!renderer || !window) {
        printf("Renderer: Invalid parameters\n");
        return false;
    }
    
    renderer->window = window;
    renderer->width = width;
    renderer->height = height;
    renderer->initialized = false;
    
    // Make the window's context current
    glfwMakeContextCurrent(window);
    
    // Enable V-Sync
    glfwSwapInterval(1);
    
    // Set up OpenGL state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Set initial viewport
    glViewport(0, 0, width, height);
    
    // Set up 2D projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);  // Top-left origin
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    renderer->initialized = true;
    printf("Renderer: Initialized %dx%d\n", width, height);
    return true;
}

void renderer_cleanup(simple_renderer_t* renderer) {
    if (!renderer || !renderer->initialized) {
        return;
    }
    
    printf("Renderer: Cleanup\n");
    renderer->initialized = false;
    renderer->window = NULL;
}

void renderer_begin_frame(simple_renderer_t* renderer) {
    if (!renderer || !renderer->initialized) {
        return;
    }
    
    glfwMakeContextCurrent(renderer->window);
    glClear(GL_COLOR_BUFFER_BIT);
}

void renderer_end_frame(simple_renderer_t* renderer) {
    if (!renderer || !renderer->initialized) {
        return;
    }
    
    glfwSwapBuffers(renderer->window);
}

void renderer_clear(simple_renderer_t* renderer, float r, float g, float b) {
    if (!renderer || !renderer->initialized) {
        return;
    }
    
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void renderer_draw_rect(simple_renderer_t* renderer, float x, float y, float width, float height, 
                       float r, float g, float b, float a) {
    if (!renderer || !renderer->initialized) {
        return;
    }
    
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
    glEnd();
}

void renderer_draw_text(simple_renderer_t* renderer, const char* text, float x, float y, 
                       float r, float g, float b) {
    if (!renderer || !renderer->initialized || !text) {
        return;
    }
    
    // Placeholder text rendering - just draw small rectangles for each character
    // In a real implementation, you'd use a font rendering library like FreeType
    glColor3f(r, g, b);
    
    float char_width = 8.0f;
    float char_height = 12.0f;
    float cursor_x = x;
    
    for (const char* c = text; *c; c++) {
        if (*c == ' ') {
            cursor_x += char_width;
            continue;
        }
        
        // Draw a simple rectangle for each character (placeholder)
        glBegin(GL_QUADS);
            glVertex2f(cursor_x, y);
            glVertex2f(cursor_x + char_width, y);
            glVertex2f(cursor_x + char_width, y + char_height);
            glVertex2f(cursor_x, y + char_height);
        glEnd();
        
        cursor_x += char_width;
    }
}

void renderer_resize(simple_renderer_t* renderer, uint32_t width, uint32_t height) {
    if (!renderer || !renderer->initialized) {
        return;
    }
    
    renderer->width = width;
    renderer->height = height;
    
    glfwMakeContextCurrent(renderer->window);
    glViewport(0, 0, width, height);
    
    // Update projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    printf("Renderer: Resized to %dx%d\n", width, height);
}