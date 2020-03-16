#pragma once

#include <vulkan/vulkan.h>

struct SwapchainDetails {
        uint32_t surface_format_count;
        uint32_t present_mode_count;
        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR *surface_formats;
        VkPresentModeKHR *present_modes;
};

void select_surface_format(uint32_t count, const VkSurfaceFormatKHR *surface_formats,
        VkSurfaceFormatKHR *surface_format);

void select_extent(const VkSurfaceCapabilitiesKHR *capabilities, uint32_t width,
        uint32_t height, VkExtent2D *extent);

void destroy_swapchain_details(struct SwapchainDetails *details);

uint32_t select_image_count(const VkSurfaceCapabilitiesKHR *capabilities);

/*
 * details should be cleaned up by destroy_swapchain_details()
 */ 
int is_swapchain_details_complete(const VkSurfaceKHR surface, const VkPhysicalDevice gpu,
        struct SwapchainDetails *details);

VkPresentModeKHR select_present_mode(uint32_t count, const VkPresentModeKHR *present_modes);