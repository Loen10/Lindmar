#pragma once

#include <memory>
#include <vector>
#include <array>
#include <vulkan/vulkan.h>

namespace lmar::render
{
#ifndef NDEBUG
    using Layers = std::array<const char*, 1>;
#endif

    class Instance
    {
    public:        
        Instance();
        ~Instance();

        void createDebugMessenger(const VkDebugUtilsMessengerCreateInfoEXT& appInfo,
            VkDebugUtilsMessengerEXT& debugMessenger) const;
        void destroyDebugMessenger(VkDebugUtilsMessengerEXT& debugMessenger) const;
    private:
        VkInstance mHandle;

        std::vector<const char*> getExtensions() const;
    #ifndef NDEBUG
        Layers getLayers() const;
    #endif
    };
}