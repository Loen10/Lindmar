#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

#include "device.h"

#define DEVICE_EXTENSION_COUNT 1
static const char* const DEVICE_EXTENSIONS[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

static int isQueueFamiliesComplete(const VkSurfaceKHR surface, const VkPhysicalDevice gpu,
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

static int isSwapchainDetailsComplete(const VkSurfaceKHR surface, const VkPhysicalDevice gpu,
        struct SwapchainDetails *details)
{
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &details->surfaceFormatCount, NULL);

        if (details->surfaceFormatCount == 0)
                return 0;
        
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &details->presentModeCount, NULL);

        if (details->presentModeCount == 0)
                return 0;

        details->surfaceFormats = malloc(sizeof(details->surfaceFormatCount));
        details->presentModes = malloc(sizeof(details->presentModeCount));

        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &details->surfaceFormatCount, details->surfaceFormats);
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &details->presentModeCount, details->presentModes);
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &details->capabilities);

        return 1;
}

static int isGpuSuitable(const VkSurfaceKHR surface, const VkPhysicalDevice gpu,
        struct QueueFamilyIndices *indices, struct SwapchainDetails *details)
{
        return isQueueFamiliesComplete(surface, gpu, indices) &&
                isDeviceExtensionsSupport(gpu) &&
                isSwapchainDetailsComplete(surface, gpu, details);
}

/*
 * Should be cleaned up by caller
 */
static VkDeviceQueueCreateInfo *getQueueCreateInfos(const struct QueueFamilyIndices *indices, uint32_t *count)
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
}