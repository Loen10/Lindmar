#include <cstring>

#include "instance.hpp"
#include "debugMessenger.hpp"
#include "Renderer.hpp"

namespace lmar::render
{
    Renderer::Renderer()
        : mWindow{window::create()}, mInstance{instance::create()}
    #ifndef NDEBUG
        , mDebugMessenger{debugMessenger::create(mInstance)}
    #endif
    {
        run();
    }

    void Renderer::run()
    {
        while (!glfwWindowShouldClose(mWindow.get()))
        {
            glfwPollEvents();
        }
    }
}