#pragma once

#include <memory>
#include <vulkan/vulkan.h>

#include "util.hpp"

#ifndef NDEBUG
namespace lmar::render
{
    class DebugMessenger
    {
    private:
        const VkInstance instance;

        VkDebugUtilsMessengerEXT handle;
    public:
        DebugMessenger(const VkInstance instance) : instance{instance},
            handle{createDebugMessenger()} {};
        ~DebugMessenger();
    private:
        VkDebugUtilsMessengerEXT createDebugMessenger() const;
        
        static VkBool32 callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageTypes,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    };
}
#endif