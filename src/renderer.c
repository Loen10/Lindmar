#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "device.h"
#include "instance.h"
#include "swapchain_details.h"
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

static void createSwapchain(const struct QueueFamilyIndices *indices, const struct SwapchainDetails *details,
        struct Renderer *renderer)
{
        VkSurfaceFormatKHR surfacefmt;
        selectSurfaceFormat(details->surfaceFormatCount, details->surfaceFormats,
                &surfacefmt);
        
        VkExtent2D extent;
        selectExtent(&details->capabilities, WIDTH, HEIGHT, &extent);

        uint32_t qindex_count = 0;
        VkSharingMode shrmode;
        uint32_t *qindices = getQueueIndices(indices, &qindex_count, &shrmode);
        VkSwapchainCreateInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = renderer->surface;
        info.queueFamilyIndexCount = qindex_count;
        info.pQueueFamilyIndices = qindices;
        info.imageSharingMode = shrmode;
        info.clipped = VK_TRUE;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.imageArrayLayers = 1;
        info.imageColorSpace = surfacefmt.colorSpace;
        info.imageExtent = extent;
        info.imageFormat = surfacefmt.format;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        info.minImageCount = selectImageCount(&details->capabilities);
        info.presentMode = selectPresentMode(details->presentModeCount, details->presentModes);
        info.preTransform = details->capabilities.currentTransform;

        vkCreateSwapchainKHR(renderer->device, &info, NULL, &renderer->swapchain);
        free(qindices);
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
        createSwapchain(&indices, &details, renderer);
}

void runRenderer(const struct Renderer *renderer)
{
        while (!glfwWindowShouldClose(renderer->window)) {
                glfwPollEvents();
        }
}

void destroyRenderer(struct Renderer *renderer)
{
        vkDestroySwapchainKHR(renderer->device, renderer->swapchain, NULL);
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