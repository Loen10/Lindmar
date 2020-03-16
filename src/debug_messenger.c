#ifndef NDEBUG
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "debug_messenger.h"

static VkBool32 callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data)
{
        printf("%s\n", callback_data->pMessage);
        return VK_FALSE;
}

void create_debug_messenger(struct Renderer *renderer)
{
        VkDebugUtilsMessengerCreateInfoEXT info = {};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
        info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        info.pfnUserCallback = &callback;

        PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(renderer->instance, "vkCreateDebugUtilsMessengerEXT");

        assert_vulkan(func(renderer->instance, &info, NULL, &renderer->debug_messenger),
                "Failed to create a Vulkan debug messenger!");
}

void destroy_debug_messenger(const struct Renderer *renderer)
{
        PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(renderer->instance, "vkDestroyDebugUtilsMessengerEXT");

        func(renderer->instance, renderer->debug_messenger, NULL);
}
#endif