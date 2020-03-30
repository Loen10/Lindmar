#pragma once

#include <glfw/glfw3.h>

#include "Window.hpp"
#include "Instance.hpp"
#include "DebugMessenger.hpp"

namespace lmar::render
{
    class Renderer
    {
    public:
        Renderer();
    private:
        Window mWindow;
        Instance mInstance;
    #ifndef NDEBUG
        DebugMessenger mDebugMessenger;
    #endif
    
        void run();
    };
}