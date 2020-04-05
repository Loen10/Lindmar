#pragma once

#include <memory>
#include <glfw/glfw3.h>

namespace lmar::render::window
{
    inline constexpr uint32_t cDefaultWidth = 1280, cDefaultHeight = 720;

    struct Deleter
    {
        inline void operator()(GLFWwindow* window) const
        {
            glfwDestroyWindow(window);
            glfwTerminate();
        }
    };

    GLFWwindow* create();
}