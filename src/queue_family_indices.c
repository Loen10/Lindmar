#include <malloc.h>
#include <vulkan/vulkan.h>

#include "queue_family_indices.h"

uint32_t *get_queue_indices(const struct QueueFamilyIndices *indices, uint32_t *count,
        VkSharingMode *sharing_mode)
{
        uint32_t *qindices;

        if (indices->graphics == indices->present) {
                *count = 0;
                *sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
                qindices = NULL;
        } else {
                *count = 2;
                *sharing_mode = VK_SHARING_MODE_CONCURRENT;
                qindices = malloc(*count * sizeof(uint32_t));
                qindices[0] = indices->graphics;
                qindices[1] = indices->present;
        }

        return qindices;    
}

int is_queue_families_complete(const VkSurfaceKHR surface, const VkPhysicalDevice gpu,
        struct QueueFamilyIndices *indices)
{
        uint32_t famcount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &famcount, NULL);

        VkQueueFamilyProperties families[famcount];
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &famcount, families);

        indices->graphics = -1;
        indices->present = -1;

        for (uint32_t i = 0; i < famcount; ++i) {
                if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                        indices->graphics = i;
                
                VkBool32 prsntsupport = 0;
                vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &prsntsupport);

                if (prsntsupport)
                        indices->present = i;
        }

        return indices->graphics >= 0 && indices->present >= 0;
}

/*
 * Should be cleaned up by caller
 */
VkDeviceQueueCreateInfo *get_queue_create_infos(const struct QueueFamilyIndices *indices, uint32_t *count)
{
        float priority = 1.0f;
        VkDeviceQueueCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.pQueuePriorities = &priority;
        info.queueCount = 1;
        info.queueFamilyIndex = indices->graphics;

        VkDeviceQueueCreateInfo *infos;

        if (indices->graphics == indices->present) {
                *count = 1;

                infos = malloc(*count * sizeof(VkDeviceQueueCreateInfo));
                infos[0] = info;
        } else {
                *count = 2;

                infos = infos = malloc(*count * sizeof(VkDeviceQueueCreateInfo));
                infos[0] = info;

                info.queueFamilyIndex = indices->present;

                infos[1] = info;
        }

        return infos;
}