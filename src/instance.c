#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "instance.h"

const char* const LAYERS[] = { "VK_LAYER_LUNARG_standard_validation" };

void createInstance(struct Renderer *renderer)
{
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.apiVersion = VK_API_VERSION_1_0;
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
        appInfo.pApplicationName = "Lindmar";
        appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
        appInfo.pEngineName = "Mountain Smithy";
       
        uint32_t extensionCount = 0;
        const char **extensions = getInstanceExtensions(&extensionCount);

#ifndef NDEBUG
        assertLayersSupport();
#endif

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = extensionCount;
        createInfo.ppEnabledExtensionNames = extensions;
#ifndef NDEBUG
        createInfo.enabledLayerCount = LAYER_COUNT;
        createInfo.ppEnabledLayerNames = LAYERS;
#endif

        assertVulkan(vkCreateInstance(&createInfo, NULL, &renderer->instance),
                "Failed to create a Vulkan instance!");

#ifndef NDEBUG
        free(extensions);
#endif
}

/*
 * Debug: Should be cleaned up by caller
 * Release: Should be cleaned up by glfwTerminate()
 */
const char **getInstanceExtensions(uint32_t *count)
{
#ifdef NDEBUG
        return glfwGetRequiredInstanceExtensions(count);
#else
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        *count = glfwExtensionCount + 1;
        const char **extensions = malloc(sizeof(*count * sizeof(const char *)));
        memcpy(extensions, glfwExtensions, glfwExtensionCount * sizeof(const char *));
        extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

        uint32_t availableExtensionCount = 0;
        vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, NULL);

        VkExtensionProperties availableExtensions[availableExtensionCount];
        vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, availableExtensions);

        for (uint32_t i = 0; i < *count; ++i) {
                for (uint32_t j = 0; j < availableExtensionCount; ++j) {
                        if (strcmp(extensions[i], availableExtensions[j].extensionName) == 0)
                                goto nextExtension;
                }

                printf("Failed to locate a required Vulkan instance extension!");
                exit(-1);

                nextExtension:;
        }

        return extensions;      
#endif
}

#ifndef NDEBUG
void assertLayersSupport() 
{
        uint32_t availableLayerCount = 0;
        vkEnumerateInstanceLayerProperties(&availableLayerCount, NULL);

        VkLayerProperties availableLayers[availableLayerCount];
        vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers);

        for (uint32_t i = 0; i < LAYER_COUNT; ++i) {
                for (uint32_t j = 0; j < availableLayerCount; ++j) {
                        if (strcmp(LAYERS[i], availableLayers[j].layerName) == 0)
                                goto nextLayer;
                }

                printf("Failed to locate a required Vulkan layer!");
                exit(-1);

                nextLayer:;
        }
}
#endif