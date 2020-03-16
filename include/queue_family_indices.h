#pragma once

#include <vulkan/vulkan.h>

struct QueueFamilyIndices {
        int graphics;
        int present;
};

/*
 * Should be cleaned up by caller
 */
uint32_t *get_queue_indices(const struct QueueFamilyIndices *indices, uint32_t *count,
        VkSharingMode *sharing_mode);

int is_queue_families_complete(const VkSurfaceKHR surface, const VkPhysicalDevice gpu,
        struct QueueFamilyIndices *indices);

VkDeviceQueueCreateInfo *get_queue_create_infos(const struct QueueFamilyIndices *indices, uint32_t *count);