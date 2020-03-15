#include <malloc.h>
#include <vulkan/vulkan.h>

#include "queue_family_indices.h"

int isQueueFamiliesComplete(const VkSurfaceKHR surface, const VkPhysicalDevice gpu,
        struct QueueFamilyIndices *indices)
{
        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &familyCount, NULL);

        VkQueueFamilyProperties families[familyCount];
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &familyCount, families);

        indices->graphics = -1;
        indices->present = -1;

        for (uint32_t i = 0; i < familyCount; ++i) {
                if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                        indices->graphics = i;
                
                VkBool32 presentSupport = 0;
                vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &presentSupport);

                if (presentSupport)
                        indices->present = i;
        }

        return indices->graphics >= 0 && indices->present >= 0;
}

/*
 * Should be cleaned up by caller
 */
VkDeviceQueueCreateInfo *getQueueCreateInfos(const struct QueueFamilyIndices *indices, uint32_t *count)
{
        float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        createInfo.pQueuePriorities = &queuePriority;
        createInfo.queueCount = 1;
        createInfo.queueFamilyIndex = indices->graphics;

        VkDeviceQueueCreateInfo *createInfos;

        if (indices->graphics == indices->present) {
                *count = 1;

                createInfos = malloc(*count * sizeof(VkDeviceQueueCreateInfo));
                createInfos[0] = createInfo;
        } else {
                *count = 2;

                createInfos = createInfos = malloc(*count * sizeof(VkDeviceQueueCreateInfo));
                createInfos[0] = createInfo;

                createInfo.queueFamilyIndex = indices->present;

                createInfos[1] = createInfo;
        }

        return createInfos;
}

uint32_t *getQueueIndices(const struct QueueFamilyIndices *indices, uint32_t *count,
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