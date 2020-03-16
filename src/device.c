#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "device.h"

#define DEVICE_EXTENSION_COUNT 1
static const char* const DEVICE_EXTENSIONS[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

static int is_device_extensions_support(const VkPhysicalDevice gpu)
{
        uint32_t extcount = 0;
        vkEnumerateDeviceExtensionProperties(gpu, NULL, &extcount, NULL);

        VkExtensionProperties exts[extcount];
        vkEnumerateDeviceExtensionProperties(gpu, NULL, &extcount, exts);

        for (uint32_t i = 0; i < DEVICE_EXTENSION_COUNT; ++i) {
                for (uint32_t j = 0; j < extcount; ++j) {
                        if (strcmp(DEVICE_EXTENSIONS[i], exts[j].extensionName) == 0)
                                goto nextExtensions;
                }

                return 0;

                nextExtensions:;
        }

        return 1;
}

/*
 * details should be cleaned up by destroy_swapchain_details()
 */
static int is_gpu_suitable(const VkSurfaceKHR surface, const VkPhysicalDevice gpu,
        struct QueueFamilyIndices *indices, struct SwapchainDetails *details)
{
        return is_queue_families_complete(surface, gpu, indices) &&
                is_device_extensions_support(gpu) &&
                is_swapchain_details_complete(surface, gpu, details);
}

/*
 * details should be cleaned up by destroy_swapchain_details()
 */
void select_gpu(struct Renderer *renderer, struct QueueFamilyIndices *indices, 
        struct SwapchainDetails *details)
{
        uint32_t gpucount = 0;
        vkEnumeratePhysicalDevices(renderer->instance, &gpucount, NULL);

        VkPhysicalDevice gpus[gpucount];
        vkEnumeratePhysicalDevices(renderer->instance, &gpucount, gpus);

        for (uint32_t i = 0; i < gpucount; ++i) {
                if (is_gpu_suitable(renderer->surface, gpus[i], indices, details)) {
                        renderer->gpu = gpus[i];
                        return;
                }     
        }

        printf("Failed to select a suitable GPU!");
        exit(-1);
}

void create_device(const struct QueueFamilyIndices *indices, struct Renderer *renderer)
{
        uint32_t qinfo_count = 0;
        VkDeviceQueueCreateInfo *qinfos = get_queue_create_infos(indices, &qinfo_count);

        VkPhysicalDeviceFeatures dvcfeatures = {};
        VkDeviceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        info.pEnabledFeatures = &dvcfeatures;
        info.enabledExtensionCount = DEVICE_EXTENSION_COUNT;
        info.ppEnabledExtensionNames = DEVICE_EXTENSIONS;
        info.queueCreateInfoCount = qinfo_count;
        info.pQueueCreateInfos = qinfos;
        
        assert_vulkan(vkCreateDevice(renderer->gpu, &info, NULL, &renderer->device),
                "Failed to create a Vulkan device");
        
        free(qinfos);
}