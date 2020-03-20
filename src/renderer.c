#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include "instance.h"
#include "renderer.h"

#define WIDTH 1280
#define HEIGHT 720
#define DEVICE_EXTENSION_COUNT 1
static const char* const DEVICE_EXTENSIONS[] = { 
        VK_KHR_SWAPCHAIN_EXTENSION_NAME 
};

struct Renderer {
        uint32_t image_count;
        GLFWwindow *window;
        VkImageView *image_views;
        VkInstance instance;
#ifndef NDEBUG
        VkDebugUtilsMessengerEXT debug_messenger;
#endif
        VkSurfaceKHR surface;
        VkPhysicalDevice gpu;
        VkDevice device;
        VkSurfaceFormatKHR surface_format;
        VkSwapchainKHR swapchain;
        VkRenderPass render_pass;
        VkPipelineLayout pipeline_layout;
        VkPipeline graphics_pipeline;
};

struct QueueFamilyIndices {
        int graphics;
        int present;
};

struct SwapchainDetails {
        uint32_t surface_format_count;
        uint32_t present_mode_count;
        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR *surface_formats;
        VkPresentModeKHR *present_modes;
};

static void assert_vulkan(VkResult res, const char *msg)
{
        if (res != VK_SUCCESS) {
                printf("%s\n", msg);
                exit(-1);
        }
}

static void create_window(struct Renderer *renderer)
{
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        renderer->window = glfwCreateWindow(WIDTH, HEIGHT, "Lindmar", NULL, NULL);
}

static void create_instance(struct Renderer *renderer)
{
        VkApplicationInfo appinfo = {};
        appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appinfo.apiVersion = VK_API_VERSION_1_0;
        appinfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
        appinfo.pApplicationName = "Lindmar";
        appinfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
        appinfo.pEngineName = "Mountain Smithy";
       
        uint32_t extcount = 0;
        const char **exts = get_instance_extensions(&extcount);

#ifndef NDEBUG
        assert_layers_support();
#endif

        VkInstanceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pApplicationInfo = &appinfo;
        info.enabledExtensionCount = extcount;
        info.ppEnabledExtensionNames = exts;
#ifndef NDEBUG
        info.enabledLayerCount = LAYER_COUNT;
        info.ppEnabledLayerNames = LAYERS;
#endif

        assert_vulkan(vkCreateInstance(&info, NULL, &renderer->instance),
                "Failed to create a Vulkan instance!");

#ifndef NDEBUG
        free(exts);
#endif
}

/* Should be cleaned up by caller */
static VkDeviceQueueCreateInfo *get_queue_create_infos(const struct QueueFamilyIndices *indices, uint32_t *count)
{
        float priority = 1.0f;
        VkDeviceQueueCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.pQueuePriorities = &priority;
        info.queueCount = 1;
        info.queueFamilyIndex = indices->graphics;

        VkDeviceQueueCreateInfo *infos;

        if (indices->graphics == indices->present) {
                *count = 1;

                infos = malloc(*count * sizeof(VkDeviceQueueCreateInfo));
                infos[0] = info;
        } else {
                *count = 2;

                infos = infos = malloc(*count * sizeof(VkDeviceQueueCreateInfo));
                infos[0] = info;

                info.queueFamilyIndex = indices->present;

                infos[1] = info;
        }

        return infos;
}

static void create_device(const struct QueueFamilyIndices *indices,
        struct Renderer *renderer)
{
        uint32_t qinfo_count = 0;
        VkDeviceQueueCreateInfo *qinfos = get_queue_create_infos(indices, &qinfo_count);

        VkPhysicalDeviceFeatures dvcfeatures = {};
        VkDeviceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        info.pEnabledFeatures = &dvcfeatures;
        info.enabledExtensionCount = DEVICE_EXTENSION_COUNT;
        info.ppEnabledExtensionNames = DEVICE_EXTENSIONS;
        info.queueCreateInfoCount = qinfo_count;
        info.pQueueCreateInfos = qinfos;
        
        assert_vulkan(vkCreateDevice(renderer->gpu, &info, NULL, &renderer->device),
                "Failed to create a Vulkan device");
        
        free(qinfos);
}

#ifndef NDEBUG
static VkBool32 debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data)
{
        printf("%s\n", callback_data->pMessage);
        return VK_FALSE;
}

static void create_debug_messenger(struct Renderer *renderer)
{
        VkDebugUtilsMessengerCreateInfoEXT info = {};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
        info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        info.pfnUserCallback = &debug_callback;

        PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(renderer->instance, "vkCreateDebugUtilsMessengerEXT");

        assert_vulkan(func(renderer->instance, &info, NULL, &renderer->debug_messenger),
                "Failed to create a Vulkan debug messenger!");
}

static void destroy_debug_messenger(const struct Renderer *renderer)
{
        PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(renderer->instance, "vkDestroyDebugUtilsMessengerEXT");

        func(renderer->instance, renderer->debug_messenger, NULL); 
}
#endif

static void create_surface(struct Renderer *renderer)
{
        assert_vulkan(glfwCreateWindowSurface(renderer->instance, renderer->window, NULL, &renderer->surface),
                "Failed to create a Vulkan surface!");
}

static void select_surface_format(uint32_t count, const VkSurfaceFormatKHR *surface_formats,
        VkSurfaceFormatKHR *surface_format)
{
        for (uint32_t i = 0; i < count; ++i) {
                if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
                        surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                        *surface_format = surface_formats[i];
                }
        }

        *surface_format = surface_formats[0];
}

static uint32_t clamp(uint32_t val, uint32_t top, uint32_t bot)
{
        if (val > top)
                return top;
        if (val < bot)
                return bot;
                
        return val;
}

static void select_extent(const VkSurfaceCapabilitiesKHR *capabilities, uint32_t width,
        uint32_t height, VkExtent2D *extent)
{
        if (capabilities->currentExtent.width != UINT32_MAX)
                *extent = capabilities->currentExtent;

        extent->width = clamp(width, capabilities->maxImageExtent.width, capabilities->minImageExtent.width);
        extent->height = clamp(height, capabilities->maxImageExtent.height, capabilities->minImageExtent.height);      
}

static uint32_t *get_queue_indices(const struct QueueFamilyIndices *indices, 
        uint32_t *count, VkSharingMode *sharing_mode)
{
        uint32_t *qindices;

        if (indices->graphics == indices->present) {
                *count = 0;
                *sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
                qindices = NULL;
        } else {
                *count = 2;
                *sharing_mode = VK_SHARING_MODE_CONCURRENT;
                qindices = malloc(*count * sizeof(uint32_t));
                qindices[0] = indices->graphics;
                qindices[1] = indices->present;
        }

        return qindices;    
}

static uint32_t select_image_count(const VkSurfaceCapabilitiesKHR *capabilities)
{
        uint32_t imgcount = capabilities->minImageCount + 1;

        if (capabilities->maxImageCount > 0 && imgcount > capabilities->maxImageCount) {
                imgcount = capabilities->maxImageCount;
        }

        return imgcount;
}

static VkPresentModeKHR select_present_mode(uint32_t count, const VkPresentModeKHR *present_modes)
{
        for (uint32_t i = 0; i < count; ++i) {
                if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
                        return present_modes[i];
        }

        return VK_PRESENT_MODE_FIFO_KHR;
}

static void create_swapchain(const struct QueueFamilyIndices *indices,
        const struct SwapchainDetails *details, struct Renderer *renderer)
{
        select_surface_format(details->surface_format_count, details->surface_formats,
                &renderer->surface_format);
        
        VkExtent2D extent;
        select_extent(&details->capabilities, WIDTH, HEIGHT, &extent);

        uint32_t qindex_count = 0;
        VkSharingMode shrmode;
        uint32_t *qindices = get_queue_indices(indices, &qindex_count, &shrmode);
        VkSwapchainCreateInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = renderer->surface;
        info.queueFamilyIndexCount = qindex_count;
        info.pQueueFamilyIndices = qindices;
        info.imageSharingMode = shrmode;
        info.clipped = VK_TRUE;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.imageArrayLayers = 1;
        info.imageColorSpace = renderer->surface_format.colorSpace;
        info.imageExtent = extent;
        info.imageFormat = renderer->surface_format.format;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        info.minImageCount = select_image_count(&details->capabilities);
        info.presentMode = select_present_mode(details->present_mode_count, details->present_modes);
        info.preTransform = details->capabilities.currentTransform;

        assert_vulkan(vkCreateSwapchainKHR(renderer->device, &info, NULL, &renderer->swapchain),
                "Failed to create a Vulkan swapchain");
        free(qindices);
}

static void create_image_views(struct Renderer *renderer)
{
        vkGetSwapchainImagesKHR(renderer->device, renderer->swapchain, &renderer->image_count, NULL);

        VkImage images[renderer->image_count];
        vkGetSwapchainImagesKHR(renderer->device, renderer->swapchain, &renderer->image_count, images);

        renderer->image_views = malloc(renderer->image_count * sizeof(VkImageView));

        for (uint32_t i = 0; i < renderer->image_count; ++i) {
                VkImageViewCreateInfo info = {};
                info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
                info.format = renderer->surface_format.format;
                info.image = images[i];
                info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                info.subresourceRange.layerCount = 1;
                info.subresourceRange.levelCount = 1;
                info.viewType = VK_IMAGE_VIEW_TYPE_2D;

                assert_vulkan(vkCreateImageView(renderer->device, &info, NULL, &renderer->image_views[i]),
                        "Failed to create a Vulkan image view!");
        }
}

static void create_render_pass(struct Renderer *renderer)
{
        VkAttachmentDescription attachment = {};
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachment.format = renderer->surface_format.format;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkAttachmentReference attchref = {};
        attchref.attachment = 0;
        attchref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &attchref;
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        VkRenderPassCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.attachmentCount = 1;
        info.pAttachments = &attachment;
        info.pSubpasses = &subpass;
        info.subpassCount = 1;

        assert_vulkan(vkCreateRenderPass(renderer->device, &info, NULL, &renderer->render_pass),
                "Failed to create a Vulkan render pass!");
}

/* Should be cleaned up by free() */
static uint32_t *get_shader_code(const char *path, size_t *size)
{
        FILE *file = fopen(path, "rb");

        if (file == NULL) {
                printf("Failed to open file at %s\n", path);
                exit(-1);
        }

        fseek(file, 0, SEEK_END);
        
        *size = ftell(file);

        rewind(file);

        size_t count = *size / 4;
        uint32_t *code = malloc(*size);
        
        if (fread(code, sizeof(uint32_t), count, file) != count) {
                printf("Failed to read file at %s\n", path);
                exit(-1);
        }
        
        fclose(file);

        return code;
}

static void create_shader_info(const char *path,
        const VkShaderStageFlagBits stage, const VkDevice device,
        VkPipelineShaderStageCreateInfo *info)
{
        size_t code_size = 0;
        uint32_t *code = get_shader_code(path, &code_size);

        VkShaderModuleCreateInfo modinfo = {};
        modinfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        modinfo.codeSize = code_size;
        modinfo.pCode = code;

        info->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info->pName = "main";
        info->stage = stage;
        info->pSpecializationInfo = NULL;
        info->pNext = NULL;
        info->flags = 0;
        assert_vulkan(vkCreateShaderModule(device, &modinfo, NULL,
                &info->module), "Failed to create a Vulkan shader module!");
        //free(code);
}

static void create_graphics_pipeline(struct Renderer *renderer)
{
        VkPipelineLayoutCreateInfo lytinfo = {};
        lytinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        assert_vulkan(vkCreatePipelineLayout(renderer->device, &lytinfo, NULL,
                &renderer->pipeline_layout),
                "Failed to create a Vulkan pipeline layout!");

        VkPipelineColorBlendAttachmentState blndattach_state = {};
        blndattach_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
                VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo blndinfo = {};
        blndinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blndinfo.attachmentCount = 1;
        blndinfo.pAttachments = &blndattach_state;

        VkPipelineInputAssemblyStateCreateInfo inptassembly_info = {};
        inptassembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inptassembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineRasterizationStateCreateInfo rstrinfo = {};
        rstrinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rstrinfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rstrinfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rstrinfo.lineWidth = 1.0f;
        rstrinfo.polygonMode = VK_POLYGON_MODE_FILL;

        VkPipelineMultisampleStateCreateInfo mltsample_info = {};
        mltsample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        mltsample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        uint32_t shdrcount = 2;
        VkPipelineShaderStageCreateInfo shdrinfos[shdrcount];
        create_shader_info("../include/shader/basic_vs.spv",
                VK_SHADER_STAGE_VERTEX_BIT, renderer->device, &shdrinfos[0]);
        create_shader_info("../include/shader/basic_fs.spv",
                VK_SHADER_STAGE_FRAGMENT_BIT, renderer->device, &shdrinfos[1]);

        VkPipelineVertexInputStateCreateInfo vrtinput_info = {};
        vrtinput_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        
        VkRect2D scissor = {};
        scissor.extent.height = HEIGHT;
        scissor.extent.width = WIDTH;

        VkViewport viewport= {};
        viewport.height = HEIGHT;
        viewport.width = WIDTH;
        viewport.maxDepth = 1.0f;

        VkPipelineViewportStateCreateInfo vwprtinfo = {};
        vwprtinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        vwprtinfo.pScissors = &scissor;
        vwprtinfo.pViewports = &viewport;
        vwprtinfo.scissorCount = 1;
        vwprtinfo.viewportCount = 1;

        VkGraphicsPipelineCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        info.layout = renderer->pipeline_layout;
        info.pColorBlendState = &blndinfo;
        info.pInputAssemblyState = &inptassembly_info;
        info.pRasterizationState = &rstrinfo;
        info.pMultisampleState = &mltsample_info;
        info.pStages = shdrinfos;
        info.pVertexInputState = &vrtinput_info;
        info.pViewportState = &vwprtinfo;
        info.renderPass = renderer->render_pass;
        info.stageCount = shdrcount;
        info.subpass = 0;

        assert_vulkan(vkCreateGraphicsPipelines(renderer->device, NULL, 1, &info, NULL,
                &renderer->graphics_pipeline),
                "Failed to create a Vulkan graphics pipeline!");

        vkDestroyShaderModule(renderer->device, shdrinfos[0].module, NULL);
        vkDestroyShaderModule(renderer->device, shdrinfos[1].module, NULL);
}

static void destroy_image_views(struct Renderer *renderer)
{
        for (uint32_t i = 0; i < renderer->image_count; ++i) {
                vkDestroyImageView(renderer->device, renderer->image_views[i], NULL);
        }

        free(renderer->image_views);
}

static void destroy_swapchain_details(struct SwapchainDetails *details)
{
        free(details->surface_formats);
        free(details->present_modes);
}

static int is_queue_families_complete(const VkSurfaceKHR surface, const VkPhysicalDevice gpu,
        struct QueueFamilyIndices *indices)
{
        uint32_t famcount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &famcount, NULL);

        VkQueueFamilyProperties families[famcount];
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &famcount, families);

        indices->graphics = -1;
        indices->present = -1;

        for (uint32_t i = 0; i < famcount; ++i) {
                if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                        indices->graphics = i;
                
                VkBool32 prsntsupport = 0;
                vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &prsntsupport);

                if (prsntsupport)
                        indices->present = i;
        }

        return indices->graphics >= 0 && indices->present >= 0;
}

static int is_device_extensions_support(const VkPhysicalDevice gpu)
{
        uint32_t extcount = 0;
        vkEnumerateDeviceExtensionProperties(gpu, NULL, &extcount, NULL);

        VkExtensionProperties exts[extcount];
        vkEnumerateDeviceExtensionProperties(gpu, NULL, &extcount, exts);

        for (uint32_t i = 0; i < DEVICE_EXTENSION_COUNT; ++i) {
                for (uint32_t j = 0; j < extcount; ++j) {
                        if (strcmp(DEVICE_EXTENSIONS[i], exts[j].extensionName) == 0)
                                goto nextExtensions;
                }

                return 0;

                nextExtensions:;
        }

        return 1;
}

static int is_swapchain_details_complete(const VkSurfaceKHR surface, const VkPhysicalDevice gpu,
        struct SwapchainDetails *details)
{
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &details->surface_format_count, NULL);

        if (details->surface_format_count == 0)
                return 0;
        
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &details->present_mode_count, NULL);

        if (details->present_mode_count == 0)
                return 0;

        details->surface_formats = malloc(sizeof(details->surface_format_count));
        details->present_modes = malloc(sizeof(details->present_mode_count));

        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &details->surface_format_count, details->surface_formats);
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &details->present_mode_count, details->present_modes);
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &details->capabilities);

        return 1;
}

/* details should be cleaned up by destroy_swapchain_details() */
static int is_gpu_suitable(const VkSurfaceKHR surface, const VkPhysicalDevice gpu,
        struct QueueFamilyIndices *indices, struct SwapchainDetails *details)
{
        return is_queue_families_complete(surface, gpu, indices) &&
                is_device_extensions_support(gpu) &&
                is_swapchain_details_complete(surface, gpu, details);
}

/* details should be cleaned up by destroy_swapchain_details() */
static void select_gpu(struct Renderer *renderer, struct QueueFamilyIndices *indices, 
        struct SwapchainDetails *details)
{
        uint32_t gpucount = 0;
        vkEnumeratePhysicalDevices(renderer->instance, &gpucount, NULL);

        VkPhysicalDevice gpus[gpucount];
        vkEnumeratePhysicalDevices(renderer->instance, &gpucount, gpus);

        for (uint32_t i = 0; i < gpucount; ++i) {
                if (is_gpu_suitable(renderer->surface, gpus[i], indices, details)) {
                        renderer->gpu = gpus[i];
                        return;
                }     
        }

        printf("Failed to select a suitable GPU!");
        exit(-1);
}

void run_renderer(const struct Renderer *renderer)
{
        while (!glfwWindowShouldClose(renderer->window)) {
                glfwPollEvents();
        }
}

void destroy_renderer(struct Renderer *renderer)
{
        vkDestroyPipeline(renderer->device, renderer->graphics_pipeline, NULL);
        vkDestroyPipelineLayout(renderer->device, renderer->pipeline_layout,
                NULL);
        vkDestroyRenderPass(renderer->device, renderer->render_pass, NULL);
        destroy_image_views(renderer);
        vkDestroySwapchainKHR(renderer->device, renderer->swapchain, NULL);
        vkDestroyDevice(renderer->device, NULL);
        vkDestroySurfaceKHR(renderer->instance, renderer->surface, NULL);
#ifndef NDEBUG
        destroy_debug_messenger(renderer);
#endif
        vkDestroyInstance(renderer->instance, NULL);
        glfwDestroyWindow(renderer->window);
        glfwTerminate();
}

struct Renderer *create_renderer()
{
        struct Renderer *renderer = malloc(sizeof(struct Renderer));
        create_window(renderer);
        create_instance(renderer);
#ifndef NDEBUG
        create_debug_messenger(renderer);
#endif
        create_surface(renderer);  

        struct QueueFamilyIndices indices;
        struct SwapchainDetails details;
        select_gpu(renderer, &indices, &details);
        create_device(&indices, renderer);
        create_swapchain(&indices, &details, renderer);
        create_image_views(renderer);
        destroy_swapchain_details(&details);
        create_render_pass(renderer);
        create_graphics_pipeline(renderer);

        return renderer;
}