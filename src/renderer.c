#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
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
        GLFWwindow *window;
        VkInstance instance;
};

static void createWindow(Renderer renderer)
{
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        renderer->window = glfwCreateWindow(WIDTH, HEIGHT, "Lindmar", NULL, NULL);
}

/*
 * Debug: Should be cleaned up by caller
 * Release: Should be cleaned up by glfwTerminate()
 */
static const char **getInstanceExtensions(uint32_t *count)
{
#ifdef NDEBUG
        return glfwGetRequiredInstanceExtensions(count);
#else
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        *count = glfwExtensionCount + 1;
        const char **extensions = malloc(sizeof(*count * sizeof(const char *)));
        memcpy(extensions, glfwExtensions, glfwExtensionCount * sizeof(const char *));
        extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

        uint32_t availableExtensionCount = 0;
        vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, NULL);

        VkExtensionProperties availableExtensions[availableExtensionCount];
        vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, availableExtensions);

        for (uint32_t i = 0; i < *count; ++i) {
                for (uint32_t j = 0; j < availableExtensionCount; ++j) {
                        if (strcmp(extensions[i], availableExtensions[j].extensionName) == 0)
                                goto nextExtension;
                }

                printf("Failed to locate a required Vulkan instance extension!");
                exit(-1);

                nextExtension:;
        }

        return extensions;      
#endif
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

        uint32_t extensionCount = 0;
        const char **extensions = getInstanceExtensions(&extensionCount);

        VkInstanceCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pApplicationInfo = &appInfo
        };

        assert_vulkan(vkCreateInstance(&createInfo, NULL, &renderer->instance),
                "Failed to create a Vulkan instance!");

#ifndef NDEBUG
        free(extensions);
#endif
}

Renderer createRenderer()
{
        Renderer renderer = malloc(sizeof(struct Renderer_T));

        createWindow(renderer);
        createInstance(renderer);

        return renderer;
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
