

#include "Window.hpp"

using namespace lmar::render;

Window::Window()
{
    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    mHandle = glfwCreateWindow(sDefaultWidth, sDefaultHeight, "Lindmar", nullptr, nullptr);
}

Window::~Window()
{
    glfwDestroyWindow(mHandle);
    glfwTerminate();
}