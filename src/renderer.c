#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <glfw/glfw3.h>
#include <vulkan/vulkan.h>

#include "renderer.h"

#define WIDTH 1280
#define HEIGHT 720
#define assert_vulkan(res, msg) \
if (res != VK_SUCCESS) { \
        printf("%s", msg); \
        exit(-1); \
}

struct Renderer_T {
        GLFWwindow* window;
        VkInstance instance;
};

static void createWindow(Renderer renderer)
{
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        renderer->window = glfwCreateWindow(WIDTH, HEIGHT, "Lindmar", NULL, NULL);
}

static void createInstance(Renderer renderer)
{
        VkApplicationInfo appInfo = {
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .apiVersion = VK_API_VERSION_1_0,
                .applicationVersion = VK_MAKE_VERSION(0, 0, 0),
                .pApplicationName = "Lindmar",
                .engineVersion = VK_MAKE_VERSION(0, 0, 0),
                .pEngineName = "Mountain Smithy"
        };

        VkInstanceCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pApplicationInfo = &appInfo
        };

        assert_vulkan(vkCreateInstance(&createInfo, NULL, &renderer->instance),
                "Failed to create a Vulkan instance!");
}

void createRenderer(Renderer* renderer)
{
        *renderer = malloc(sizeof(struct Renderer_T));

        createWindow(*renderer);
        createInstance(*renderer);
}

void runRenderer(const Renderer renderer)
{
        while (!glfwWindowShouldClose(renderer->window)) {
                glfwPollEvents();
        }
}

void destroyRenderer(const Renderer renderer)
{
        vkDestroyInstance(renderer->instance, NULL);
        glfwDestroyWindow(renderer->window);
        glfwTerminate();
        free(renderer);
}
