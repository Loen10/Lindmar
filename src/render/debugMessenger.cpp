#include <iostream>

#include "debugMessenger.hpp"

PFN_vkCreateDebugUtilsMessengerEXT pfnCreate;
PFN_vkDestroyDebugUtilsMessengerEXT pfnDestroy;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger)
{
    return pfnCreate(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator)
{
    return pfnDestroy(instance, messenger, pAllocator);
}

namespace
{
    VkBool32 callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        std::cerr << pCallbackData->pMessage << std::endl;
        return false;
    }
}

namespace lmar::render::debugMessenger
{
    vk::UniqueDebugUtilsMessengerEXT create(const vk::UniqueInstance& instance)
    {
        vk::DebugUtilsMessengerCreateInfoEXT createInfo{{},
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation, &callback};

        pfnCreate = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            instance->getProcAddr("vkCreateDebugUtilsMessengerEXT"));

        pfnDestroy = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            instance->getProcAddr("vkDestroyDebugUtilsMessengerEXT"));

        return instance->createDebugUtilsMessengerEXTUnique(createInfo);
    }
}