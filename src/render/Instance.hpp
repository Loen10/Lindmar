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

        std::unique_ptr<VkInstance_T, util::InstanceDeleter> handle;
    public:
        Instance() : handle{createInstance()} {};
    private:
        std::vector<const char*> getInstanceExtensions() const;
        VkInstance createInstance() const;
    #ifndef NDEBUG
        void initLayers(Layers& layers) const;
    #endif
    };
}