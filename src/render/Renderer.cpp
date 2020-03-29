#include <cstring>

#include "Renderer.hpp"

using namespace lmar::render;

Renderer::Renderer() : window{createWindow()}
#ifndef NDEBUG
    , debugMessenger{instance},
#endif
{
    run();
}

void Renderer::run()
{
    while (!glfwWindowShouldClose(window.get()))
    {
        glfwPollEvents();
    }
}

GLFWwindow* Renderer::createWindow() const
{
    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    return glfwCreateWindow(defaultWidth, defaultHeight, "Lindmar", nullptr, nullptr);
}