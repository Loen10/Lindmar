#pragma once

#include "Instance.hpp"

#ifndef NDEBUG
namespace lmar::render
{
    class DebugMessenger
    {
    public:
        DebugMessenger(const Instance& instance);
        ~DebugMessenger();
    private:        
        const Instance& mInstance;
        VkDebugUtilsMessengerEXT mHandle;

        static VkBool32 callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageTypes,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    };
}
#endif