#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <glfw/glfw3.h>
#include <vulkan/vulkan.h>

#include "renderer.h"

#define WIDTH 1280
#define HEIGHT 720
#define assert_vulkan(res, msg) if (res != VK_SUCCESS) { printf("%s\n", msg); exit(-1); }

struct Renderer_T {
        GLFWwindow *window;
        VkInstance instance;
#ifndef NDEBUG
        VkDebugUtilsMessengerEXT debugMessenger;
#endif
};

static void createWindow(Renderer renderer)
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
/*
 * Should be cleaned up by caller
 */
static const char **getLayers(uint32_t *count) 
{
        *count = 1;
        const char **layers = malloc(*count * sizeof(const char *));
        layers[0] = "VK_LAYER_LUNARG_standard_validation";

        uint32_t availableLayerCount = 0;
        vkEnumerateInstanceLayerProperties(&availableLayerCount, NULL);

        VkLayerProperties availableLayers[availableLayerCount];
        vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers);

        for (int i = 0; i < *count; ++i) {
                for (int j = 0; j < availableLayerCount; ++j) {
                        if (strcmp(layers[i], availableLayers[j].layerName) == 0)
                                goto nextLayer;
                }

                printf("Failed to locate a required Vulkan layer!");
                exit(-1);

                nextLayer:;
        }

        return layers;
}
#endif

static void createInstance(Renderer renderer)
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
        uint32_t layerCount = 0;
        const char **layers = getLayers(&layerCount);         
#endif

        VkInstanceCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pApplicationInfo = &appInfo,
                .enabledExtensionCount = extensionCount,
                .ppEnabledExtensionNames = extensions,
        #ifndef NDEBUG
                .enabledLayerCount = layerCount,
                .ppEnabledLayerNames = layers
        #endif
        };

        assert_vulkan(vkCreateInstance(&createInfo, NULL, &renderer->instance),
                "Failed to create a Vulkan instance!");

#ifndef NDEBUG
        free(extensions);
        free(layers);
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

static void createDebugMessenger(Renderer renderer)
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

static void destroyDebugMessenger(const Renderer renderer)
{
        PFN_vkDestroyDebugUtilsMessengerEXT destroyFunc = (PFN_vkDestroyDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(renderer->instance, "vkDestroyDebugUtilsMessengerEXT");

        destroyFunc(renderer->instance, renderer->debugMessenger, NULL);
}
#endif

Renderer createRenderer()
{
        Renderer renderer = malloc(sizeof(struct Renderer_T));

        createWindow(renderer);
        createInstance(renderer);
#ifndef NDEBUG
        createDebugMessenger(renderer);
#endif

        return renderer;
}

void runRenderer(const Renderer renderer)
{
        while (!glfwWindowShouldClose(renderer->window)) {
                glfwPollEvents();
        }
}

void destroyRenderer(const Renderer renderer)
{
#ifndef NDEBUG
        destroyDebugMessenger(renderer);
#endif
        vkDestroyInstance(renderer->instance, NULL);
        glfwDestroyWindow(renderer->window);
        glfwTerminate();
        free(renderer);
}
