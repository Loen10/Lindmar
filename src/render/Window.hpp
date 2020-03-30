#pragma once

#include <glfw/glfw3.h>

namespace lmar::render
{
    class Window
    {
    public:
        static constexpr uint32_t sDefaultWidth = 1280, sDefaultHeight = 720;
        
        Window();
        ~Window();

        inline bool shouldClose() { return glfwWindowShouldClose(mHandle); };
    private:
        GLFWwindow* mHandle;
    };
}