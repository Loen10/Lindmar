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

        inline operator std::shared_ptr<VkInstance_T>() { return mHandle; };
    private:
        std::shared_ptr<VkInstance_T> mHandle;

        std::vector<const char*> getExtensions() const;
        VkInstance createInstance() const;
    #ifndef NDEBUG
        Layers getLayers() const;
    #endif
    };
}