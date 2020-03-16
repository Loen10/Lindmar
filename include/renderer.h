#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

struct Renderer {
        uint32_t image_count;
        GLFWwindow *window;
        VkImageView *image_views;
        VkInstance instance;
#ifndef NDEBUG
        VkDebugUtilsMessengerEXT debug_messenger;
#endif
        VkSurfaceKHR surface;
        VkPhysicalDevice gpu;
        VkDevice device;
        VkSurfaceFormatKHR surface_format;
        VkSwapchainKHR swapchain;
};

/*
 * renderer should be cleaned up by destroy_renderer()
 */
void create_renderer(struct Renderer *renderer);

void run_renderer(const struct Renderer *renderer);

void destroy_renderer(struct Renderer *renderer);

void assert_vulkan(VkResult res, const char *msg);