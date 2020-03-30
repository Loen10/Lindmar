#pragma once

#include "Instance.hpp"

#ifndef NDEBUG
namespace lmar::render
{
    class DebugMessenger
    {
    public:
        DebugMessenger(const std::shared_ptr<VkInstance_T>& instance);
        ~DebugMessenger();
    private:
        std::shared_ptr<VkInstance_T> mInstance;
        VkDebugUtilsMessengerEXT mHandle;

        static VkBool32 callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageTypes,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    };
}
#endif