#pragma once

#include <vector>
#include <array>
#include <vulkan/vulkan.h>
#include <glfw/glfw3.h>

#include <memory>

#include "util.hpp"
#include "Instance.hpp"
#include "DebugMessenger.hpp"

namespace lmar::render
{
    class Renderer
    {
    private:
        static constexpr uint32_t defaultWidth = 1280, defaultHeight = 720;

        std::unique_ptr<GLFWwindow, util::WindowDeleter> window;
        Instance instance;
    #ifndef NDEBUG
        DebugMessenger debugMessenger;
    #endif
    public:
        Renderer();

        void run();
    private:
        GLFWwindow* createWindow() const;
    };
}