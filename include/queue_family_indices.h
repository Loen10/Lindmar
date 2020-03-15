#pragma once

#include <vulkan/vulkan.h>

struct QueueFamilyIndices {
        int graphics;
        int present;
};

int isQueueFamiliesComplete(const VkSurfaceKHR surface, const VkPhysicalDevice gpu,
        struct QueueFamilyIndices *indices);

VkDeviceQueueCreateInfo *getQueueCreateInfos(const struct QueueFamilyIndices *indices, uint32_t *count);
/*
 * Should be cleaned up by caller
 */
uint32_t *getQueueIndices(const struct QueueFamilyIndices *indices, uint32_t *count,
        VkSharingMode *sharing_mode);