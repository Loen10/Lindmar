#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "device.h"
#include "instance.h"
#include "debug_messenger.h"
#include "renderer.h"

#define WIDTH 1280
#define HEIGHT 720

static void createWindow(struct Renderer *renderer)
{
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        renderer->window = glfwCreateWindow(WIDTH, HEIGHT, "Lindmar", NULL, NULL);
}

static void createSurface(struct Renderer *renderer)
{
        assertVulkan(glfwCreateWindowSurface(renderer->instance, renderer->window, NULL, &renderer->surface),
                "Failed to create a Vulkan surface!");
}

void createRenderer(struct Renderer *renderer)
{
        createWindow(renderer);
        createInstance(renderer);
#ifndef NDEBUG
        createDebugMessenger(renderer);
#endif
        createSurface(renderer);

        struct QueueFamilyIndices indices;
        struct SwapchainDetails details;
        selectGpu(renderer, &indices, &details);
        createDevice(&indices, renderer);
}

void runRenderer(const struct Renderer *renderer)
{
        while (!glfwWindowShouldClose(renderer->window)) {
                glfwPollEvents();
        }
}

void destroyRenderer(struct Renderer *renderer)
{
        vkDestroyDevice(renderer->device, NULL);
        vkDestroySurfaceKHR(renderer->instance, renderer->surface, NULL);
#ifndef NDEBUG
        destroyDebugMessenger(renderer);
#endif
        vkDestroyInstance(renderer->instance, NULL);
        glfwDestroyWindow(renderer->window);
        glfwTerminate();
}

void assertVulkan(VkResult res, const char *msg)
{
        if (res != VK_SUCCESS) {
                printf("%s\n", msg);
                exit(-1);
        }
}