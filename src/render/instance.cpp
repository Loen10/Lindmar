#include <array>
#include <vector>
#include <glfw/glfw3.h>

#include "instance.hpp"

namespace
{
    std::vector<const char*> getExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> required{glfwExtensions, glfwExtensions + glfwExtensionCount};
    #ifndef NDEBUG
        required.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    #endif

        auto available = vk::enumerateInstanceExtensionProperties();

        for (const auto& req : required)
        {
            for (const auto& abl : available)
            {
                if (std::strcmp(req, abl.extensionName) == 0)
                {
                    goto nextRequired;
                }
            }

            throw std::runtime_error{"Failed to locate a required Vulkan instance extension!"};

            nextRequired:;
        }

        return required;
    }

#ifndef NDEBUG
    using Layers = std::array<const char*, 1>;
    Layers getLayers()
    {
        constexpr Layers required{"VK_LAYER_LUNARG_standard_validation"};
        auto available = vk::enumerateInstanceLayerProperties();

        for (const auto& req : required)
        {
            for (const auto& abl : available)
            {
                if (std::strcmp(req, abl.layerName) == 0)
                {
                    goto nextLayer;
                }
            }

            throw std::runtime_error{"Failed to locate a required Vulkan layer!"};

            nextLayer:;
        }

        return required;
    }
#endif
}

namespace lmar::render::instance
{
    vk::UniqueInstance create()
    {
        vk::ApplicationInfo appInfo{"Lindmar", VK_MAKE_VERSION(0, 0, 0), "Mountain Smithy",
            VK_MAKE_VERSION(0, 2, 0), VK_API_VERSION_1_0};
        auto extensions = getExtensions();
    #ifndef NDEBUG
        auto layers = getLayers();
    #endif
        vk::InstanceCreateInfo createInfo{{}, &appInfo,
        #ifdef NDEBUG
            0U, nullptr,
        #else
            static_cast<uint32_t>(layers.size()), layers.data(),
        #endif
            static_cast<uint32_t>(extensions.size()), extensions.data()};

        return vk::createInstanceUnique(createInfo);
    }
}