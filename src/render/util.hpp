#pragma once

#include <iostream>

#include <vulkan/vulkan.h>
#include <glfw/glfw3.h>

namespace lmar::render::util
{
    struct WindowDeleter
    {
        void operator()(GLFWwindow* window) { glfwDestroyWindow(window); };
    };

    struct InstanceDeleter
    {
        void operator()(VkInstance instance) { vkDestroyInstance(instance, nullptr); };
    };

    void assertVulkan(VkResult res, const char* msg);
}