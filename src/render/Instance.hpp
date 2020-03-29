#pragma once

#include <vector>
#include <array>
#include <memory>
#include <vulkan/vulkan.h>

#include "util.hpp"

namespace lmar::render
{
    class Instance
    {
    private:
    #ifndef NDEBUG
        using Layers = std::array<const char*, 1>;
    #endif

        VkInstance handle;
    public:
        Instance() : handle{createInstance()} {};
        ~Instance() { vkDestroyInstance(handle, nullptr); };

        operator VkInstance() const { return handle; };
    private:
        std::vector<const char*> getInstanceExtensions() const;
        VkInstance createInstance() const;
    #ifndef NDEBUG
        void initLayers(Layers& layers) const;
    #endif
    };
}