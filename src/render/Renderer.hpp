#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <memory>

#include "util.hpp"

namespace lmar::render
{
    class Renderer
    {
    private:
        static constexpr uint32_t defaultWidth = 1280, defaultHeight = 720;

        std::unique_ptr<GLFWwindow, util::WindowDeleter> window;
        std::unique_ptr<VkInstance_T, util::InstanceDeleter> instance;
    public:
        Renderer();

        void run();
    private:
        GLFWwindow* createWindow() const;
        VkInstance createInstance() const;
    };
}