#ifndef NDEBUG
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "debug_messenger.h"

static VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData)
{
        printf("%s\n", callbackData->pMessage);
        return VK_FALSE;
}

void createDebugMessenger(struct Renderer *renderer)
{
        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        createInfo.pfnUserCallback = &debugCallback;

        PFN_vkCreateDebugUtilsMessengerEXT createFunc = (PFN_vkCreateDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(renderer->instance, "vkCreateDebugUtilsMessengerEXT");

        assertVulkan(createFunc(renderer->instance, &createInfo, NULL, &renderer->debugMessenger),
                "Failed to create a Vulkan debug messenger!");
}

void destroyDebugMessenger(const struct Renderer *renderer)
{
        PFN_vkDestroyDebugUtilsMessengerEXT destroyFunc = (PFN_vkDestroyDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(renderer->instance, "vkDestroyDebugUtilsMessengerEXT");

        destroyFunc(renderer->instance, renderer->debugMessenger, NULL);
}
#endif