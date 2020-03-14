#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include "renderer.h"

#define WIDTH 1280
#define HEIGHT 720
#define assert_vulkan(res, msg) if (res != VK_SUCCESS) { printf("%s\n", msg); exit(-1); }

#ifndef NDEBUG
#define LAYER_COUNT 1
static const char* const LAYERS[] = { "VK_LAYER_LUNARG_standard_validation" };
#endif

#define DEVICE_EXTENSION_COUNT 1
static const char* const DEVICE_EXTENSIONS[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

struct Renderer {
        GLFWwindow *window;
        VkInstance instance;
#ifndef NDEBUG
        VkDebugUtilsMessengerEXT debugMessenger;
#endif
        VkSurfaceKHR surface;
        VkPhysicalDevice gpu;
        VkDevice device;
};

struct QueueFamilyIndices {
        int graphics;
        int present;
};

struct SwapchainDetails {
        uint32_t surfaceFormatCount;
        uint32_t presentModeCount;
        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR *surfaceFormats;
        VkPresentModeKHR *presentModes;
};

static void createWindow(struct Renderer *renderer)
{
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        renderer->window = glfwCreateWindow(WIDTH, HEIGHT, "Lindmar", NULL, NULL);
}

/*
 * Debug: Should be cleaned up by caller
 * Release: Should be cleaned up by glfwTerminate()
 */
static const char **getInstanceExtensions(uint32_t *count)
{
#ifdef NDEBUG
        return glfwGetRequiredInstanceExtensions(count);
#else
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        *count = glfwExtensionCount + 1;
        const char **extensions = malloc(sizeof(*count * sizeof(const char *)));
        memcpy(extensions, glfwExtensions, glfwExtensionCount * sizeof(const char *));
        extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

        uint32_t availableExtensionCount = 0;
        vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, NULL);

        VkExtensionProperties availableExtensions[availableExtensionCount];
        vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, availableExtensions);

        for (uint32_t i = 0; i < *count; ++i) {
                for (uint32_t j = 0; j < availableExtensionCount; ++j) {
                        if (strcmp(extensions[i], availableExtensions[j].extensionName) == 0)
                                goto nextExtension;
                }

                printf("Failed to locate a required Vulkan instance extension!");
                exit(-1);

                nextExtension:;
        }

        return extensions;      
#endif
}

#ifndef NDEBUG
static void assertLayersSupport() 
{
        uint32_t availableLayerCount = 0;
        vkEnumerateInstanceLayerProperties(&availableLayerCount, NULL);

        VkLayerProperties availableLayers[availableLayerCount];
        vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers);

        for (uint32_t i = 0; i < LAYER_COUNT; ++i) {
                for (uint32_t j = 0; j < availableLayerCount; ++j) {
                        if (strcmp(LAYERS[i], availableLayers[j].layerName) == 0)
                                goto nextLayer;
                }

                printf("Failed to locate a required Vulkan layer!");
                exit(-1);

                nextLayer:;
        }
}
#endif

static void createInstance(struct Renderer *renderer)
{
        VkApplicationInfo appInfo = {
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .apiVersion = VK_API_VERSION_1_0,
                .applicationVersion = VK_MAKE_VERSION(0, 0, 0),
                .pApplicationName = "Lindmar",
                .engineVersion = VK_MAKE_VERSION(0, 0, 0),
                .pEngineName = "Mountain Smithy"
        };

        uint32_t extensionCount = 0;
        const char **extensions = getInstanceExtensions(&extensionCount);

#ifndef NDEBUG
        assertLayersSupport();
#endif

        VkInstanceCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pApplicationInfo = &appInfo,
                .enabledExtensionCount = extensionCount,
                .ppEnabledExtensionNames = extensions,
        #ifndef NDEBUG
                .enabledLayerCount = LAYER_COUNT,
                .ppEnabledLayerNames = LAYERS
        #endif
        };

        assert_vulkan(vkCreateInstance(&createInfo, NULL, &renderer->instance),
                "Failed to create a Vulkan instance!");

#ifndef NDEBUG
        free(extensions);
#endif
}

#ifndef NDEBUG
static VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData)
{
        printf("%s\n", callbackData->pMessage);
        return VK_FALSE;
}

static void createDebugMessenger(struct Renderer *renderer)
{
        VkDebugUtilsMessengerCreateInfoEXT createInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
                .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
                .pfnUserCallback = &debugCallback
        };

        PFN_vkCreateDebugUtilsMessengerEXT createFunc = (PFN_vkCreateDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(renderer->instance, "vkCreateDebugUtilsMessengerEXT");

        assert_vulkan(createFunc(renderer->instance, &createInfo, NULL, &renderer->debugMessenger),
                "Failed to create a Vulkan debug messenger!");
}

static void destroyDebugMessenger(const struct Renderer *renderer)
{
        PFN_vkDestroyDebugUtilsMessengerEXT destroyFunc = (PFN_vkDestroyDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(renderer->instance, "vkDestroyDebugUtilsMessengerEXT");

        destroyFunc(renderer->instance, renderer->debugMessenger, NULL);
}
#endif

static void createSurface(struct Renderer *renderer)
{
        assert_vulkan(glfwCreateWindowSurface(renderer->instance, renderer->window, NULL, &renderer->surface),
                "Failed to create a Vulkan surface!");
}

static int isQueueFamiliesComplete(const VkSurfaceKHR surface, const VkPhysicalDevice gpu,
        struct QueueFamilyIndices *indices)
{
        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &familyCount, NULL);

        VkQueueFamilyProperties families[familyCount];
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &familyCount, families);

        indices->graphics = -1;
        indices->present = -1;

        for (uint32_t i = 0; i < familyCount; ++i) {
                if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                        indices->graphics = i;
                
                VkBool32 presentSupport = 0;
                vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &presentSupport);

                if (presentSupport)
                        indices->present = i;
        }

        return indices->graphics >= 0 && indices->present >= 0;
}

static int isDeviceExtensionsSupport(const VkPhysicalDevice gpu)
{
        uint32_t availableExtensionCount = 0;
        vkEnumerateDeviceExtensionProperties(gpu, NULL, &availableExtensionCount, NULL);

        VkExtensionProperties availableExtensions[availableExtensionCount];
        vkEnumerateDeviceExtensionProperties(gpu, NULL, &availableExtensionCount, availableExtensions);

        for (uint32_t i = 0; i < DEVICE_EXTENSION_COUNT; ++i) {
                for (uint32_t j = 0; j < availableExtensionCount; ++j) {
                        if (strcmp(DEVICE_EXTENSIONS[i], availableExtensions[j].extensionName) == 0)
                                goto nextExtensions;
                }

                return 0;

                nextExtensions:;
        }

        return 1;
}

static int isSwapchainDetailsComplete(const VkSurfaceKHR surface, const VkPhysicalDevice gpu,
        struct SwapchainDetails *details)
{
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &details->surfaceFormatCount, NULL);

        if (details->surfaceFormatCount == 0)
                return 0;
        
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &details->presentModeCount, NULL);

        if (details->presentModeCount == 0)
                return 0;

        details->surfaceFormats = malloc(sizeof(details->surfaceFormatCount));
        details->presentModes = malloc(sizeof(details->presentModeCount));

        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &details->surfaceFormatCount, details->surfaceFormats);
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &details->presentModeCount, details->presentModes);
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &details->capabilities);

        return 1;
}

static int isGpuSuitable(const VkSurfaceKHR surface, const VkPhysicalDevice gpu,
        struct QueueFamilyIndices *indices, struct SwapchainDetails *details)
{
        return isQueueFamiliesComplete(surface, gpu, indices) &&
                isDeviceExtensionsSupport(gpu) &&
                isSwapchainDetailsComplete(surface, gpu, details);
}

static void selectGpu(struct Renderer *renderer, struct QueueFamilyIndices *indices, 
        struct SwapchainDetails *details)
{
        uint32_t gpuCount = 0;
        vkEnumeratePhysicalDevices(renderer->instance, &gpuCount, NULL);

        VkPhysicalDevice gpus[gpuCount];
        vkEnumeratePhysicalDevices(renderer->instance, &gpuCount, gpus);

        for (uint32_t i = 0; i < gpuCount; ++i) {
                if (isGpuSuitable(renderer->surface, gpus[i], indices, details)) {
                        renderer->gpu = gpus[gpuCount];
                        return;
                }     
        }

        printf("Failed to select a suitable GPU!");
        exit(-1);
}

struct Renderer *createRenderer()
{
        struct Renderer *renderer = malloc(sizeof(struct Renderer));
        createWindow(renderer);
        createInstance(renderer);
#ifndef NDEBUG
        createDebugMessenger(renderer);
#endif
        createSurface(renderer);

        struct QueueFamilyIndices indices;
        struct SwapchainDetails details;
        selectGpu(renderer, &indices, &details);

        return renderer;
}

void runRenderer(const struct Renderer *renderer)
{
        while (!glfwWindowShouldClose(renderer->window)) {
                glfwPollEvents();
        }
}

void destroyRenderer(struct Renderer *renderer)
{
        vkDestroySurfaceKHR(renderer->instance, renderer->surface, NULL);
#ifndef NDEBUG
        destroyDebugMessenger(renderer);
#endif
        vkDestroyInstance(renderer->instance, NULL);
        glfwDestroyWindow(renderer->window);
        glfwTerminate();
        free(renderer);
}