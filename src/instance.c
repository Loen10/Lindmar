#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include "instance.h"

void print_exit(const char *message)
{
    fprintf(stderr, "%s\n", message);
    exit(-1);
}

#ifndef NDEBUG
void get_layers(const char *layers[LAYER_COUNT]) 
{
    layers[0] = "VK_LAYER_LUNARG_standard_validation";

    uint32_t lyrcount = 0;
    vkEnumerateInstanceLayerProperties(&lyrcount, NULL);

    VkLayerProperties lyrs[lyrcount];
    vkEnumerateInstanceLayerProperties(&lyrcount, lyrs);

    for (uint32_t i = 0; i < LAYER_COUNT; ++i) {
        for (uint32_t j = 0; j < lyrcount; ++j)
            if (strcmp(layers[i], lyrs[j].layerName) == 0) 
                goto nextLayer; 

        print_exit("Failed to locate a required Vulkan layer!");

        nextLayer:;
    }
}
#endif

const char **create_instance_extensions(uint32_t *count) 
{
#ifdef NDEBUG
    return glfwGetRequiredInstanceExtensions(count);
#else
    uint32_t glfwext_count = 0;
    const char **glfwexts = glfwGetRequiredInstanceExtensions(&glfwext_count);
    *count = glfwext_count + 1;
    const char **reqextensions = malloc(sizeof(*count * sizeof(const char *)));
    memcpy(reqextensions, glfwexts, glfwext_count * sizeof(const char *));
    reqextensions[glfwext_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    uint32_t extcount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extcount, NULL);

    VkExtensionProperties exts[extcount];
    vkEnumerateInstanceExtensionProperties(NULL, &extcount, exts);

    for (uint32_t i = 0; i < *count; ++i) {
        for (uint32_t j = 0; j < extcount; ++j)
            if (strcmp(reqextensions[i], exts[j].extensionName) == 0)
                goto next_extensions;

        print_exit("Failed to locate a required Vulkan instance extension!");

        next_extensions:;
    }

    return reqextensions;      
#endif
}