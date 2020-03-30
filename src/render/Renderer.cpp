

#include "Renderer.hpp"

using namespace lmar::render;

Renderer::Renderer()
#ifndef NDEBUG
    : mDebugMessenger{mInstance}
#endif
{
    run();
}

void Renderer::run()
{
    while (!mWindow.shouldClose())
    {
        glfwPollEvents();
    }
}