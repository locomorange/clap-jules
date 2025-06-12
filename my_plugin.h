#pragma once

#include <clap/clap.h>
#include <imgui.h>

// Forward declare Vulkan handles if not including vulkan.h directly everywhere
// Or ensure vulkan.h is included before this header if it defines types used here.
// For simplicity here, we'll assume vulkan.h will be included before this in my_plugin.cpp
struct VkInstance_T;
struct VkPhysicalDevice_T;
struct VkDevice_T;
struct VkQueue_T;
struct VkRenderPass_T;
struct VkCommandPool_T;
struct VkCommandBuffer_T;
struct VkDescriptorPool_T;

typedef struct {
    clap_plugin_t plugin;
    // ImGui context for the plugin instance
    ImGuiContext* imgui_context;

    // GUI related state
    bool gui_created;
    bool gui_is_visible;
    clap_window_t parent_window; // To store parent window details

    // Vulkan specific data (placeholders, to be managed by Vulkan backend)
    // These would typically be provided by the host or initialized by the plugin
    // based on host information.
    VkInstance_T*       vulkan_instance;
    VkPhysicalDevice_T* vulkan_physical_device;
    VkDevice_T*         vulkan_device;
    VkQueue_T*          vulkan_queue;
    uint32_t            vulkan_queue_family_index;
    VkRenderPass_T*     vulkan_render_pass; // ImGui will render to this pass
    VkCommandPool_T*    vulkan_command_pool;
    VkCommandBuffer_T*  vulkan_command_buffer; // For ImGui rendering commands
    VkDescriptorPool_T* vulkan_descriptor_pool; // For ImGui's Vulkan backend
    // Add other necessary Vulkan objects like framebuffers, image views etc. as needed.
    // For example, a minimal set of swapchain/framebuffer related handles:
    // uint32_t            min_image_count;
    // uint32_t            current_frame_index;
    // VkFramebuffer*      framebuffers; // Array
    // VkImageView*        image_views;  // Array

    // Add any other plugin-specific data here
} my_plugin_t;

// Plugin factory ID
extern const CLAP_EXPORT struct clap_plugin_factory my_plugin_factory;

// Function to get the plugin factory (for cross-platform compatibility)
CLAP_EXPORT const struct clap_plugin_factory* get_plugin_factory();
