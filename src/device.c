#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

#include "device.h"

#define DEVICE_EXTENSION_COUNT 1
static const char* const DEVICE_EXTENSIONS[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

static int isDeviceExtensionsSupport(const VkPhysicalDevice gpu)
{
        uint32_t availableExtensionCount = 0;
        vkEnumerateDeviceExtensionProperties(gpu, NULL, &availableExtensionCount, NULL);

        VkExtensionProperties availableExtensions[availableExtensionCount];
        vkEnumerateDeviceExtensionProperties(gpu, NULL, &availableExtensionCount, availableExtensions);

        for (uint32_t i = 0; i < DEVICE_EXTENSION_COUNT; ++i) {
                for (uint32_t j = 0; j < availableExtensionCount; ++j) {
                        if (strcmp(DEVICE_EXTENSIONS[i], availableExtensions[j].extensionName) == 0)
                                goto nextExtensions;
                }

                return 0;

                nextExtensions:;
        }

        return 1;
}

static int isGpuSuitable(const VkSurfaceKHR surface, const VkPhysicalDevice gpu,
        struct QueueFamilyIndices *indices, struct SwapchainDetails *details)
{
        return isQueueFamiliesComplete(surface, gpu, indices) &&
                isDeviceExtensionsSupport(gpu) &&
                isSwapchainDetailsComplete(surface, gpu, details);
}

void selectGpu(struct Renderer *renderer, struct QueueFamilyIndices *indices, 
        struct SwapchainDetails *details)
{
        uint32_t gpuCount = 0;
        vkEnumeratePhysicalDevices(renderer->instance, &gpuCount, NULL);

        VkPhysicalDevice gpus[gpuCount];
        vkEnumeratePhysicalDevices(renderer->instance, &gpuCount, gpus);

        for (uint32_t i = 0; i < gpuCount; ++i) {
                if (isGpuSuitable(renderer->surface, gpus[i], indices, details)) {
                        renderer->gpu = gpus[i];
                        return;
                }     
        }

        printf("Failed to select a suitable GPU!");
        exit(-1);
}

void createDevice(const struct QueueFamilyIndices *indices, struct Renderer *renderer)
{
        uint32_t queueInfoCount = 0;
        VkDeviceQueueCreateInfo *queueInfos = getQueueCreateInfos(indices, &queueInfoCount);

        VkPhysicalDeviceFeatures deviceFeatures = {};
        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = DEVICE_EXTENSION_COUNT;
        createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS;
        createInfo.queueCreateInfoCount = queueInfoCount;
        createInfo.pQueueCreateInfos = queueInfos;
        
        assertVulkan(vkCreateDevice(renderer->gpu, &createInfo, NULL, &renderer->device),
                "Failed to create a Vulkan device");
        
        free(queueInfos);
}