

#include "Renderer.hpp"

using namespace lmar::render;

Renderer::Renderer() : window{createWindow()}, instance{createInstance()}
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

VkInstance Renderer::createInstance() const
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 1);
    appInfo.pApplicationName = "Lindmar";
    appInfo.pEngineName = "Mountain Smithy";
    
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    VkInstance instance;
    util::assertVulkan(vkCreateInstance(&createInfo, nullptr, &instance),
        "Failed to create a Vulkan instance");

    return instance;
}