#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

struct Renderer {
        GLFWwindow *window;
        VkInstance instance;
#ifndef NDEBUG
        VkDebugUtilsMessengerEXT debugMessenger;
#endif
        VkSurfaceKHR surface;
        VkPhysicalDevice gpu;
        VkDevice device;
        VkSwapchainKHR swapchain;
};

void createRenderer(struct Renderer *renderer);
void runRenderer(const struct Renderer *renderer);
void destroyRenderer(struct Renderer *renderer);
void assertVulkan(VkResult res, const char *msg);