#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
struct GLFWwindow;

// Simple 2D graphics renderer
typedef struct {
    struct GLFWwindow* window;
    uint32_t width;
    uint32_t height;
    bool initialized;
} simple_renderer_t;

// Initialize the renderer
bool renderer_init(simple_renderer_t* renderer, struct GLFWwindow* window, uint32_t width, uint32_t height);

// Cleanup the renderer
void renderer_cleanup(simple_renderer_t* renderer);

// Begin rendering frame
void renderer_begin_frame(simple_renderer_t* renderer);

// End rendering frame and present
void renderer_end_frame(simple_renderer_t* renderer);

// Clear the screen with a color (RGB values 0-1)
void renderer_clear(simple_renderer_t* renderer, float r, float g, float b);

// Draw a filled rectangle
void renderer_draw_rect(simple_renderer_t* renderer, float x, float y, float width, float height, 
                       float r, float g, float b, float a);

// Draw text (placeholder - would need a font system for proper implementation)
void renderer_draw_text(simple_renderer_t* renderer, const char* text, float x, float y, 
                       float r, float g, float b);

// Resize the renderer viewport
void renderer_resize(simple_renderer_t* renderer, uint32_t width, uint32_t height);

#ifdef __cplusplus
}
#endif