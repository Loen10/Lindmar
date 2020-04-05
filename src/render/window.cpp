

#include "window.hpp"

namespace lmar::render::window
{
    GLFWwindow* create()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        return glfwCreateWindow(cDefaultWidth, cDefaultHeight, "Lindmar", nullptr, nullptr);
    }
}