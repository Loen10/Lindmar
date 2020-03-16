#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "device.h"
#include "instance.h"
#include "swapchain_details.h"
#include "debug_messenger.h"
#include "renderer.h"

#define WIDTH 1280
#define HEIGHT 720

static void create_window(struct Renderer *renderer)
{
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        renderer->window = glfwCreateWindow(WIDTH, HEIGHT, "Lindmar", NULL, NULL);
}

static void create_surface(struct Renderer *renderer)
{
        assert_vulkan(glfwCreateWindowSurface(renderer->instance, renderer->window, NULL, &renderer->surface),
                "Failed to create a Vulkan surface!");
}

static void create_swapchain(const struct QueueFamilyIndices *indices, const struct SwapchainDetails *details,
        struct Renderer *renderer)
{
        select_surface_format(details->surface_format_count, details->surface_formats,
                &renderer->surface_format);
        
        VkExtent2D extent;
        select_extent(&details->capabilities, WIDTH, HEIGHT, &extent);

        uint32_t qindex_count = 0;
        VkSharingMode shrmode;
        uint32_t *qindices = get_queue_indices(indices, &qindex_count, &shrmode);
        VkSwapchainCreateInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = renderer->surface;
        info.queueFamilyIndexCount = qindex_count;
        info.pQueueFamilyIndices = qindices;
        info.imageSharingMode = shrmode;
        info.clipped = VK_TRUE;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.imageArrayLayers = 1;
        info.imageColorSpace = renderer->surface_format.colorSpace;
        info.imageExtent = extent;
        info.imageFormat = renderer->surface_format.format;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        info.minImageCount = select_image_count(&details->capabilities);
        info.presentMode = select_present_mode(details->present_mode_count, details->present_modes);
        info.preTransform = details->capabilities.currentTransform;

        assert_vulkan(vkCreateSwapchainKHR(renderer->device, &info, NULL, &renderer->swapchain),
                "Failed to create a Vulkan swapchain");
        free(qindices);
}

/*
 * renderer->image_views should be cleaned up by destroy_image_views()
 */
static void create_image_views(struct Renderer *renderer)
{
        vkGetSwapchainImagesKHR(renderer->device, renderer->swapchain, &renderer->image_count, NULL);

        VkImage images[renderer->image_count];
        vkGetSwapchainImagesKHR(renderer->device, renderer->swapchain, &renderer->image_count, images);

        renderer->image_views = malloc(renderer->image_count * sizeof(VkImageView));

        for (uint32_t i = 0; i < renderer->image_count; ++i) {
                VkImageViewCreateInfo info = {};
                info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
                info.format = renderer->surface_format.format;
                info.image = images[i];
                info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                info.subresourceRange.layerCount = 1;
                info.subresourceRange.levelCount = 1;
                info.viewType = VK_IMAGE_VIEW_TYPE_2D;

                assert_vulkan(vkCreateImageView(renderer->device, &info, NULL, &renderer->image_views[i]),
                        "Failed to create a Vulkan image view!");
        }
}

static void destroy_image_views(struct Renderer *renderer)
{
        for (uint32_t i = 0; i < renderer->image_count; ++i) {
                vkDestroyImageView(renderer->device, renderer->image_views[i], NULL);
        }

        free(renderer->image_views);
}

void create_renderer(struct Renderer *renderer)
{
        create_window(renderer);
        create_instance(renderer);
#ifndef NDEBUG
        create_debug_messenger(renderer);
#endif
        create_surface(renderer);

        struct QueueFamilyIndices indices;
        struct SwapchainDetails details;
        select_gpu(renderer, &indices, &details);
        create_device(&indices, renderer);
        create_swapchain(&indices, &details, renderer);
        create_image_views(renderer);
        destroy_swapchain_details(&details);
}

void run_renderer(const struct Renderer *renderer)
{
        while (!glfwWindowShouldClose(renderer->window)) {
                glfwPollEvents();
        }
}

void destroy_renderer(struct Renderer *renderer)
{
        destroy_image_views(renderer);
        vkDestroySwapchainKHR(renderer->device, renderer->swapchain, NULL);
        vkDestroyDevice(renderer->device, NULL);
        vkDestroySurfaceKHR(renderer->instance, renderer->surface, NULL);
#ifndef NDEBUG
        destroy_debug_messenger(renderer);
#endif
        vkDestroyInstance(renderer->instance, NULL);
        glfwDestroyWindow(renderer->window);
        glfwTerminate();
}

void assert_vulkan(VkResult res, const char *msg)
{
        if (res != VK_SUCCESS) {
                printf("%s\n", msg);
                exit(-1);
        }
}