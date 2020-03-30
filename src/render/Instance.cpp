#include <cstring>
#include <glfw/glfw3.h>

#include "Util.hpp"
#include "Instance.hpp"

using namespace lmar::render;

Instance::Instance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.pApplicationName = "Lindmar";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 2, 0);
    appInfo.pEngineName = "Mountain Smithy";

    auto extensions = getExtensions();
#ifndef NDEBUG
    auto layers = getLayers();
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

    AssertVulkan(vkCreateInstance(&createInfo, nullptr, &mHandle),
        "Failed to create a Vulkan instance!");
}

Instance::~Instance()
{
    vkDestroyInstance(mHandle, nullptr);
}

void Instance::createDebugMessenger(const VkDebugUtilsMessengerCreateInfoEXT& appInfo,
    VkDebugUtilsMessengerEXT& debugMessenger) const
{
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(mHandle,"vkCreateDebugUtilsMessengerEXT"));
    AssertVulkan(func(mHandle, &appInfo, nullptr, &debugMessenger),
        "Failed to create a Vulkan debug messenger!");
}

void Instance::destroyDebugMessenger(VkDebugUtilsMessengerEXT& debugMessenger) const
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(mHandle,"vkDestroyDebugUtilsMessengerEXT"));
    func(mHandle, debugMessenger, nullptr);
}

std::vector<const char*> Instance::getExtensions() const
{
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions{glfwExtensions, glfwExtensions + glfwExtensionCount};

#ifndef NDEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    uint32_t availableExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions{availableExtensionCount};
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount,
        availableExtensions.data());
    
    for (const auto& extension : extensions)
    {
        for (const auto& availableExtension : availableExtensions)
        {
            if (std::strcmp(extension, availableExtension.extensionName) == 0)
            {
                goto nextExtension;
            }
        }

        throw std::runtime_error{"Failed to locate a required Vulkan instance extension!"};

        nextExtension:;
    }
#endif

    return extensions;
}

#ifndef NDEBUG
Layers Instance::getLayers() const
{
    constexpr Layers layers{"VK_LAYER_LUNARG_standard_validation"};

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

    return layers;
}
#endif