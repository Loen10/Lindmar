#pragma once

#include <vulkan/vulkan.h>

struct SwapchainDetails {
        uint32_t surfaceFormatCount;
        uint32_t presentModeCount;
        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR *surfaceFormats;
        VkPresentModeKHR *presentModes;
};

int isSwapchainDetailsComplete(const VkSurfaceKHR surface, const VkPhysicalDevice gpu,
        struct SwapchainDetails *details);
void selectSurfaceFormat(uint32_t count, const VkSurfaceFormatKHR *surface_formats,
        VkSurfaceFormatKHR *surface_format);
VkPresentModeKHR selectPresentMode(uint32_t count, const VkPresentModeKHR *present_modes);
void selectExtent(const VkSurfaceCapabilitiesKHR *capabilities, uint32_t width,
        uint32_t height, VkExtent2D *extent);
uint32_t selectImageCount(const VkSurfaceCapabilitiesKHR *capabilities);