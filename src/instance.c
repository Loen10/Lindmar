#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include "instance.h"

const char* const LAYERS[] = { "VK_LAYER_LUNARG_standard_validation" };

#ifndef NDEBUG
void assert_layers_support() 
{
        uint32_t lyrcount = 0;
        vkEnumerateInstanceLayerProperties(&lyrcount, NULL);

        VkLayerProperties lyrs[lyrcount];
        vkEnumerateInstanceLayerProperties(&lyrcount, lyrs);

        for (uint32_t i = 0; i < LAYER_COUNT; ++i) {
                for (uint32_t j = 0; j < lyrcount; ++j) {
                        if (strcmp(LAYERS[i], lyrs[j].layerName) == 0)
                                goto nextLayer;
                }

                printf("Failed to locate a required Vulkan layer!");
                exit(-1);

                nextLayer:;
        }
}
#endif

/*
 * Debug: Should be cleaned up by caller
 * Release: Should be cleaned up by glfwTerminate()
 */
const char **get_instance_extensions(uint32_t *count)
{
#ifdef NDEBUG
        return glfwGetRequiredInstanceExtensions(count);
#else
        uint32_t glfwext_count = 0;
        const char **glfwexts = glfwGetRequiredInstanceExtensions(&glfwext_count);

        *count = glfwext_count + 1;
        const char **extensions = malloc(sizeof(*count * sizeof(const char *)));
        memcpy(extensions, glfwexts, glfwext_count * sizeof(const char *));
        extensions[glfwext_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

        uint32_t extcount = 0;
        vkEnumerateInstanceExtensionProperties(NULL, &extcount, NULL);

        VkExtensionProperties exts[extcount];
        vkEnumerateInstanceExtensionProperties(NULL, &extcount, exts);

        for (uint32_t i = 0; i < *count; ++i) {
                for (uint32_t j = 0; j < extcount; ++j) {
                        if (strcmp(extensions[i], exts[j].extensionName) == 0)
                                goto next_extensions;
                }

                printf("Failed to locate a required Vulkan instance extension!");
                exit(-1);

                next_extensions:;
        }

        return extensions;      
#endif
}