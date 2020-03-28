#include <cstring>

#include "Instance.hpp"

using namespace lmar::render;

std::vector<const char*> Instance::getInstanceExtensions() const
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions{glfwExtensions,
        glfwExtensions + glfwExtensionCount};

#ifndef NDEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    uint32_t availableExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions{availableExtensionCount};
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

    for (const auto& extension : extensions)
    {
        for (const auto& availableExtension : availableExtensions)
        {
            if (std::strcmp(extension, availableExtension.extensionName) == 0)
            {
                goto nextExtension;
            }
        }

        throw std::runtime_error{"Failed to locate a required Vulkan instance extensions!"};

        nextExtension:;
    }
#endif

    return extensions;
}

VkInstance Instance::createInstance() const
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 1);
    appInfo.pApplicationName = "Lindmar";
    appInfo.pEngineName = "Mountain Smithy";
    
    auto extensions = getInstanceExtensions();
#ifndef NDEBUG
    Layers layers;
    initLayers(layers);
#endif

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
#ifndef NDEBUG
    createInfo.enabledLayerCount = layers.size();
    createInfo.ppEnabledLayerNames = layers.data();
#endif

    VkInstance instance;
    util::assertVulkan(vkCreateInstance(&createInfo, nullptr, &instance),
        "Failed to create a Vulkan instance");

    return instance;
}

#ifndef NDEBUG
void Instance::initLayers(Layers& layers) const
{
    layers[0] = "VK_LAYER_LUNARG_standard_validation";

    uint32_t availableLayerCount = 0;
    vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers{availableLayerCount};
    vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data());

    for (const auto& layer : layers)
    {
        for (const auto& availableLayer : availableLayers)
        {
            if (std::strcmp(layer, availableLayer.layerName) == 0)
            {
                goto nextLayer;
            }
        }

        throw std::runtime_error{"Failed to locate a required Vulkan layer!"};

        nextLayer:;
    }
}
#endif