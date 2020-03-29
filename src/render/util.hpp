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

    void assertVulkan(VkResult res, const char* msg);
}