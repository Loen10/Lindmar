#ifndef NDEBUG
#include <iostream>

#include "Util.hpp"
#include "DebugMessenger.hpp"

using namespace lmar::render;

DebugMessenger::DebugMessenger(const std::shared_ptr<VkInstance_T>& instance)
    : mInstance{instance}
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    createInfo.pfnUserCallback = &callback;

    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(mInstance.get(), "vkCreateDebugUtilsMessengerEXT"));
    AssertVulkan(func(mInstance.get(), &createInfo, nullptr, &mHandle),
        "Failed to create a Vulkan debug messenger!");
}

DebugMessenger::~DebugMessenger()
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(mInstance.get(), "vkDestroyDebugUtilsMessengerEXT"));
    func(mInstance.get(), mHandle, nullptr);
}

VkBool32 DebugMessenger::callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    std::cerr << pCallbackData->pMessage << std::endl;
    return false;
}
#endif