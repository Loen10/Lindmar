#include <malloc.h>

#include "swapchain_details.h"

static uint32_t clamp(uint32_t val, uint32_t top, uint32_t bot)
{
        return val > top ? top : val < bot ? bot : val;
}

void select_surface_format(uint32_t count, const VkSurfaceFormatKHR *surface_formats,
        VkSurfaceFormatKHR *surface_format)
{
        for (uint32_t i = 0; i < count; ++i) {
                if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
                        surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                        *surface_format = surface_formats[i];
                }
        }

        *surface_format = surface_formats[0];
}

void select_extent(const VkSurfaceCapabilitiesKHR *capabilities, uint32_t width,
        uint32_t height, VkExtent2D *extent)
{
        if (capabilities->currentExtent.width != UINT32_MAX)
                *extent = capabilities->currentExtent;

        extent->width = clamp(width, capabilities->maxImageExtent.width, capabilities->minImageExtent.width);
        extent->height = clamp(height, capabilities->maxImageExtent.height, capabilities->minImageExtent.height);      
}

void destroy_swapchain_details(struct SwapchainDetails *details)
{
        free(details->surface_formats);
        free(details->present_modes);
}

uint32_t select_image_count(const VkSurfaceCapabilitiesKHR *capabilities)
{
        uint32_t imgcount = capabilities->minImageCount + 1;

        if (capabilities->maxImageCount > 0 && imgcount > capabilities->maxImageCount) {
                imgcount = capabilities->maxImageCount;
        }

        return imgcount;
}

int is_swapchain_details_complete(const VkSurfaceKHR surface, const VkPhysicalDevice gpu,
        struct SwapchainDetails *details)
{
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &details->surface_format_count, NULL);

        if (details->surface_format_count == 0)
                return 0;
        
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &details->present_mode_count, NULL);

        if (details->present_mode_count == 0)
                return 0;

        details->surface_formats = malloc(sizeof(details->surface_format_count));
        details->present_modes = malloc(sizeof(details->present_mode_count));

        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &details->surface_format_count, details->surface_formats);
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &details->present_mode_count, details->present_modes);
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &details->capabilities);

        return 1;
}

VkPresentModeKHR select_present_mode(uint32_t count, const VkPresentModeKHR *present_modes)
{
        for (uint32_t i = 0; i < count; ++i) {
                if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
                        return present_modes[i];
        }

        return VK_PRESENT_MODE_FIFO_KHR;
}