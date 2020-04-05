#pragma once

#include <memory>
#include <vector>
#include <array>
#include <glfw/glfw3.h>
#include <vulkan/vulkan.hpp>

#include "window.hpp"

namespace lmar::render
{
    class Renderer
    {
    public:
        static constexpr uint32_t sDefaultWidth = 1280;
        static constexpr uint32_t sDefaultHeight = 720;

        Renderer();
    private:
        std::unique_ptr<GLFWwindow, window::Deleter> mWindow;
        vk::UniqueInstance mInstance;
    #ifndef NDEBUG
        vk::UniqueDebugUtilsMessengerEXT mDebugMessenger;
    #endif

        void run();
    };
}