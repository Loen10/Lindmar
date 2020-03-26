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
static const char* const DEVICE_EXTENSIONS[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

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

struct Renderer {
    GLFWwindow *window;
    VkInstance instance;
#ifndef NDEBUG
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
    VkSurfaceKHR surface;
    struct QueueFamilyIndices queue_families;
    struct SwapchainDetails swapchain_details;
    VkPhysicalDevice gpu;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkCommandPool command_pool;
    VkSurfaceFormatKHR surface_format;
    VkExtent2D extent;
    uint32_t image_count;
    VkSwapchainKHR swapchain;
    VkImageView *image_views;
    VkRenderPass render_pass;
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;
    VkCommandBuffer *command_buffers;
    VkFramebuffer *framebuffers;
    VkSemaphore rendered_semaphore;
    VkSemaphore presented_semaphore;
    VkFence *fences;
};

// renderer->window should be cleaned up by glfwDestroyWindow()
static void create_window(struct Renderer *renderer) 
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    renderer->window = glfwCreateWindow(WIDTH, HEIGHT, "Lindmar", NULL, NULL);
}

static void assert_vulkan(VkResult res, const char *msg) 
{
    if (res != VK_SUCCESS) {
        printf("%s\n", msg);
        exit(-1);
    }
}

// renderer->instance should be cleaned up by vkDestroyInstance()
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

#ifndef NDEBUG
static VkBool32 debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data)
{
    printf("%s\n", callback_data->pMessage);
    return VK_FALSE;
}

// renderer->debug_messenger should be cleaned up by destroy_debug_messenger()
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

    PFN_vkCreateDebugUtilsMessengerEXT func = 
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        renderer->instance, "vkCreateDebugUtilsMessengerEXT");

    assert_vulkan(func(renderer->instance, &info, NULL, &renderer->debug_messenger),
        "Failed to create a Vulkan debug messenger!");
}

static void destroy_debug_messenger(const struct Renderer *renderer)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = 
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        renderer->instance, "vkDestroyDebugUtilsMessengerEXT");

    func(renderer->instance, renderer->debug_messenger, NULL); 
}
#endif

// renderer->surface should be cleaned up by vkDestroySurfaceKHR()
static void create_surface(struct Renderer *renderer)
{
    assert_vulkan(glfwCreateWindowSurface(renderer->instance, renderer->window, NULL,
        &renderer->surface), "Failed to create a Vulkan surface!");
}

static int is_queue_families_complete(struct Renderer *renderer)
{
    uint32_t famcount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(renderer->gpu, &famcount, NULL);

    VkQueueFamilyProperties families[famcount];
    vkGetPhysicalDeviceQueueFamilyProperties(renderer->gpu, &famcount, families);

    renderer->queue_families.graphics = -1;
    renderer->queue_families.present = -1;

    for (uint32_t i = 0; i < famcount; ++i) {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            renderer->queue_families.graphics = i;
        
        VkBool32 presentspprt = 0;
        vkGetPhysicalDeviceSurfaceSupportKHR(renderer->gpu, i, renderer->surface, &presentspprt);

        if (presentspprt)
            renderer->queue_families.present = i;
    }

    return renderer->queue_families.graphics >= 0 && renderer->queue_families.present >= 0;
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

// details should be cleaned up by destroy_swapchain_details()
static int is_swapchain_details_complete(struct Renderer *renderer)
{
    vkGetPhysicalDeviceSurfaceFormatsKHR(renderer->gpu, renderer->surface,
        &renderer->swapchain_details.surface_format_count, NULL);

    if (renderer->swapchain_details.surface_format_count == 0)
        return 0;
    
    vkGetPhysicalDeviceSurfacePresentModesKHR(renderer->gpu, renderer->surface,
        &renderer->swapchain_details.present_mode_count, NULL);

    if (renderer->swapchain_details.present_mode_count == 0)
        return 0;

    renderer->swapchain_details.surface_formats = malloc(
        sizeof(renderer->swapchain_details.surface_format_count));
    renderer->swapchain_details.present_modes = malloc(
        sizeof(renderer->swapchain_details.present_mode_count));

    vkGetPhysicalDeviceSurfaceFormatsKHR(renderer->gpu, renderer->surface,
        &renderer->swapchain_details.surface_format_count,
        renderer->swapchain_details.surface_formats);
    vkGetPhysicalDeviceSurfacePresentModesKHR(renderer->gpu, renderer->surface,
        &renderer->swapchain_details.present_mode_count,
        renderer->swapchain_details.present_modes);
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(renderer->gpu, renderer->surface,
        &renderer->swapchain_details.capabilities);

    return 1;
}

static void destroy_swapchain_details(struct SwapchainDetails *details)
{
    free(details->surface_formats);
    free(details->present_modes);
}

// details should be cleaned up by destroy_swapchain_details()
static int is_gpu_suitable(struct Renderer *renderer)
{
    return is_queue_families_complete(renderer) && is_device_extensions_support(renderer->gpu) &&
        is_swapchain_details_complete(renderer);
}

// details should be cleaned up by destroy_swapchain_details()
static void select_gpu(struct Renderer *renderer)
{
    uint32_t gpucount = 0;
    vkEnumeratePhysicalDevices(renderer->instance, &gpucount, NULL);

    VkPhysicalDevice gpus[gpucount];
    vkEnumeratePhysicalDevices(renderer->instance, &gpucount, gpus);

    for (uint32_t i = 0; i < gpucount; ++i) {
        renderer->gpu = gpus[i];

        if (is_gpu_suitable(renderer))
            return;
    }

    printf("Failed to select a suitable GPU!");
    exit(-1);
}

// Should be cleaned up by free()
static VkDeviceQueueCreateInfo *get_queue_create_infos(const struct QueueFamilyIndices *indices,
    uint32_t *count)
{
    *count = 1;

    float priority = 1.0f;
    VkDeviceQueueCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    info.pQueuePriorities = &priority;
    info.queueCount = 1;
    info.queueFamilyIndex = indices->graphics;

    VkDeviceQueueCreateInfo *infos;

    if (indices->graphics == indices->present) {
        infos = malloc(*count * sizeof(VkDeviceQueueCreateInfo));
        infos[0] = info;
    } else {
        *count = 2;
        infos = malloc(*count * sizeof(VkDeviceQueueCreateInfo));
        infos[0] = info;
        infos[1] = info;
        infos[1].queueFamilyIndex = indices->present;
    }

    return infos;
}

// renderer->device should be cleaned up by vkDestroyDevice()
static void create_device(struct Renderer *renderer)
{
    uint32_t qinfo_count = 0;
    VkDeviceQueueCreateInfo *qinfos = get_queue_create_infos(&renderer->queue_families,
        &qinfo_count);

    VkPhysicalDeviceFeatures features = {};
    VkDeviceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    info.pEnabledFeatures = &features;
    info.enabledExtensionCount = DEVICE_EXTENSION_COUNT;
    info.ppEnabledExtensionNames = DEVICE_EXTENSIONS;
    info.queueCreateInfoCount = qinfo_count;
    info.pQueueCreateInfos = qinfos;
    
    assert_vulkan(vkCreateDevice(renderer->gpu, &info, NULL, &renderer->device),
        "Failed to create a Vulkan device");
    vkGetDeviceQueue(renderer->device, renderer->queue_families.graphics, 0,
        &renderer->graphics_queue);
    vkGetDeviceQueue(renderer->device, renderer->queue_families.present, 0,
        &renderer->present_queue);
    free(qinfos);
}

// renderer->command_pool should be cleaned up by destroy_command_pool()
static void create_command_pool(struct Renderer *renderer)
{
    VkCommandPoolCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.queueFamilyIndex = renderer->queue_families.graphics;

    assert_vulkan(vkCreateCommandPool(renderer->device, &info, NULL, &renderer->command_pool),
        "Failed to create a Vulkan command pool!");
}

static void select_surface_format(const struct SwapchainDetails *details,
    VkSurfaceFormatKHR *surface_format)
{
    for (uint32_t i = 0; i < details->surface_format_count; ++i) {
            if (details->surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
                    details->surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    *surface_format = details->surface_formats[i];
    }

    *surface_format = details->surface_formats[0];
}

static uint32_t clamp(uint32_t val, uint32_t top, uint32_t bot)
{
    if (val > top)
        return top;
    if (val < bot)
        return bot;
            
    return val;
}

static void select_extent(const VkSurfaceCapabilitiesKHR *capabilities, VkExtent2D *extent)
{
    if (capabilities->currentExtent.width != UINT32_MAX)
        *extent = capabilities->currentExtent;

    extent->width = clamp(WIDTH, capabilities->maxImageExtent.width,
        capabilities->minImageExtent.width);
    extent->height = clamp(HEIGHT, capabilities->maxImageExtent.height,
        capabilities->minImageExtent.height);      
}

// Should be cleaned up free()
static uint32_t *get_queue_indices(const struct QueueFamilyIndices *indices, uint32_t *count,
    VkSharingMode *sharing_mode)
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

    if (capabilities->maxImageCount > 0 &&
        imgcount > capabilities->maxImageCount)
        imgcount = capabilities->maxImageCount;

    return imgcount;
}

static VkPresentModeKHR select_present_mode(struct SwapchainDetails *details)
{
    for (uint32_t i = 0; i < details->present_mode_count; ++i)
        if (details->present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            return details->present_modes[i];

    return VK_PRESENT_MODE_FIFO_KHR;
}

// renderer->swapchain should be cleaned up by vkDestroySwapchain()
static void create_swapchain(struct Renderer *renderer)
{
    select_surface_format(&renderer->swapchain_details, &renderer->surface_format);
    select_extent(&renderer->swapchain_details.capabilities, &renderer->extent);

    VkSwapchainCreateInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface = renderer->surface;
    info.pQueueFamilyIndices = get_queue_indices(&renderer->queue_families,
        &info.queueFamilyIndexCount, &info.imageSharingMode);
    info.clipped = VK_TRUE;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.imageArrayLayers = 1;
    info.imageColorSpace = renderer->surface_format.colorSpace;
    info.imageExtent = renderer->extent;
    info.imageFormat = renderer->surface_format.format;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    info.minImageCount = select_image_count(&renderer->swapchain_details.capabilities);
    info.presentMode = select_present_mode(&renderer->swapchain_details);
    info.preTransform = renderer->swapchain_details.capabilities.currentTransform;

    assert_vulkan(vkCreateSwapchainKHR(renderer->device, &info, NULL, &renderer->swapchain),
            "Failed to create a Vulkan swapchain");
    free(info.pQueueFamilyIndices);
}

// renderer->image_views should be cleaned up by destroy_image_views()
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

static void destroy_image_views(struct Renderer *renderer)
{
    for (uint32_t i = 0; i < renderer->image_count; ++i) 
        vkDestroyImageView(renderer->device, renderer->image_views[i], NULL);

    free(renderer->image_views);
}

// renderer->render_pass should be cleaned up by vkDestroyRenderPass()
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

    VkSubpassDependency depend = {};
    depend.srcSubpass = VK_SUBPASS_EXTERNAL;
    depend.dstSubpass = 0;
    depend.srcAccessMask = 0;
    depend.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    depend.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    depend.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

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
    info.dependencyCount = 1;
    info.pDependencies = &depend;

    assert_vulkan(vkCreateRenderPass(renderer->device, &info, NULL, &renderer->render_pass),
        "Failed to create a Vulkan render pass!");
}

// Should be cleaned up by free()
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

// infos should be cleaned up by destroy_shader_infos()
static void create_shader_infos(const VkDevice device, VkPipelineShaderStageCreateInfo infos[2])
{
    const char *paths[2] = { 
        "../include/shader/basic_vs.spv",
        "../include/shader/basic_fs.spv" 
    };

    const VkShaderStageFlagBits stages[2] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT
    };

    for (uint32_t i = 0; i < 2; ++i) {
        size_t code_size = 0;
        uint32_t *code = get_shader_code(paths[i], &code_size);
        VkShaderModuleCreateInfo mdlinfo = {};
        mdlinfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        mdlinfo.codeSize = code_size;
        mdlinfo.pCode = code;

        infos[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        infos[i].flags = 0;
        infos[i].pName = "main";
        infos[i].pNext = NULL;
        infos[i].pSpecializationInfo = NULL;
        infos[i].stage = stages[i];
        assert_vulkan(vkCreateShaderModule(device, &mdlinfo, NULL, &infos[i].module),
            "Failed to create a Vulkan shader module!");
        free(code);
    }
}

static void destroy_shader_infos(const VkDevice device,
    VkPipelineShaderStageCreateInfo infos[2])
{    
    vkDestroyShaderModule(device, infos[0].module, NULL);
    vkDestroyShaderModule(device, infos[1].module, NULL);    
}

/* 
* renderer->pipeline_layout should be cleaned up by vkDestroyPipelineLayout()
* renderer->graphics_pipeline should be cleaned up by vkDestroyGraphicsPipeline()
*/
static void create_graphics_pipeline(struct Renderer *renderer)
{
    VkPipelineLayoutCreateInfo lytinfo = {};
    lytinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    assert_vulkan(vkCreatePipelineLayout(renderer->device, &lytinfo, NULL,
            &renderer->pipeline_layout),
            "Failed to create a Vulkan pipeline layout!");

    VkPipelineColorBlendAttachmentState blndattach_state = {};
    blndattach_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

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
    create_shader_infos(renderer->device, shdrinfos);

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
        &renderer->graphics_pipeline), "Failed to create a Vulkan graphics pipeline!");

    vkDestroyShaderModule(renderer->device, shdrinfos[0].module, NULL);
    vkDestroyShaderModule(renderer->device, shdrinfos[1].module, NULL);
}

// renderer->command_buffers should be cleaned up by destroy_command_buffers
static void create_command_buffers(struct Renderer *renderer)
{
    renderer->command_buffers = malloc(renderer->image_count * sizeof(VkCommandBuffer));
    VkCommandBufferAllocateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandBufferCount = renderer->image_count;
    info.commandPool = renderer->command_pool;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    
    assert_vulkan(vkAllocateCommandBuffers(renderer->device, &info, renderer->command_buffers),
        "Failed to allocate Vulkan command buffers!");
}

static void destroy_command_buffers(struct Renderer *renderer)
{
    vkFreeCommandBuffers(renderer->device, renderer->command_pool, renderer->image_count,
        renderer->command_buffers);
    free(renderer->command_buffers);
}

// renderer->framebuffers should be cleaned up by destroy_framebuffers()
static void create_framebuffers(struct Renderer *renderer)
{
    renderer->framebuffers = malloc(renderer->image_count * sizeof(VkFramebuffer));
    
    for (uint32_t i = 0; i < renderer->image_count; ++i) {
        VkFramebufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.attachmentCount = 1;
        info.height = renderer->extent.height;
        info.width = renderer->extent.width;
        info.layers = 1;
        info.pAttachments = &renderer->image_views[i];
        info.renderPass = renderer->render_pass;

        assert_vulkan(vkCreateFramebuffer(renderer->device, &info, NULL,
            &renderer->framebuffers[i]), "Failed to create a Vulkan framebuffer!");
    }
}

static void destroy_framebuffers(struct Renderer *renderer)
{
    for (uint32_t i = 0; i < renderer->image_count; ++i)
        vkDestroyFramebuffer(renderer->device, renderer->framebuffers[i], NULL);

    free(renderer->framebuffers);
}

static void record_command_buffers(struct Renderer *renderer) 
{
    for (uint32_t i = 0; i < renderer->image_count; ++i) {
        VkCommandBufferBeginInfo bgninfo = {};
        bgninfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        
        const VkCommandBuffer buffer = renderer->command_buffers[i];

        assert_vulkan(vkBeginCommandBuffer(buffer, &bgninfo),
            "Failed to begin a Vulkan command buffer!");

        VkClearValue clear = {0.0f, 0.0f, 0.0f, 1.0f};
        VkRenderPassBeginInfo rndrbegin = {};
        rndrbegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rndrbegin.clearValueCount = 1;
        rndrbegin.framebuffer = renderer->framebuffers[i];
        rndrbegin.pClearValues = &clear;
        rndrbegin.renderArea.extent = renderer->extent;
        rndrbegin.renderArea.offset.x = 0;
        rndrbegin.renderArea.offset.y = 0;
        rndrbegin.renderPass = renderer->render_pass;

        vkCmdBeginRenderPass(buffer, &rndrbegin, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->graphics_pipeline);
        vkCmdDraw(buffer, 3, 1, 0, 0);
        vkCmdEndRenderPass(buffer);
        assert_vulkan(vkEndCommandBuffer(buffer), "Failed to end a Vulkan command buffer!");
    }
}

// Should be cleaned up by destroy_sync_objects()
static void create_sync_objects(struct Renderer *renderer)
{      
    renderer->fences = malloc(renderer->image_count * sizeof(VkFence));

    VkSemaphoreCreateInfo seminfo = {};
    seminfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    assert_vulkan(vkCreateSemaphore(renderer->device, &seminfo, NULL,
        &renderer->rendered_semaphore), "Failed to create a Vulkan image acquired semaphore!");
    assert_vulkan(vkCreateSemaphore(renderer->device, &seminfo, NULL,
        &renderer->presented_semaphore), "Failed to create a Vulkan render finished semaphore!");

    VkFenceCreateInfo feninfo = {};
    feninfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    feninfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for (uint32_t i = 0; i < renderer->image_count; ++i)
        assert_vulkan(vkCreateFence(renderer->device, &feninfo, NULL, &renderer->fences[i]), 
            "Failed to create a Vulkan fence!");
}

static void destroy_sync_objects(struct Renderer *renderer)
{
    vkDestroySemaphore(renderer->device, renderer->rendered_semaphore, NULL);
    vkDestroySemaphore(renderer->device, renderer->presented_semaphore, NULL);

    for (uint32_t i = 0; i < renderer->image_count; ++i)
        vkDestroyFence(renderer->device, renderer->fences[i], NULL);

    free(renderer->fences);
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
    select_gpu(renderer);
    create_device(renderer);
    create_command_pool(renderer);
    create_swapchain(renderer);
    create_image_views(renderer);
    create_render_pass(renderer);
    create_graphics_pipeline(renderer);
    create_command_buffers(renderer);
    create_framebuffers(renderer);
    record_command_buffers(renderer);
    create_sync_objects(renderer);

    return renderer;
}

static void destroy_swapchain_objects(struct Renderer *renderer)
{
    destroy_framebuffers(renderer);
    destroy_command_buffers(renderer);
    vkDestroyPipelineLayout(renderer->device, renderer->pipeline_layout, NULL);
    vkDestroyPipeline(renderer->device, renderer->graphics_pipeline, NULL);
    vkDestroyRenderPass(renderer->device, renderer->render_pass, NULL);
    destroy_image_views(renderer);
    vkDestroySwapchainKHR(renderer->device, renderer->swapchain, NULL);
}

static void recreate_swapchain_objects(struct Renderer *renderer)
{
    vkDeviceWaitIdle(renderer->device);
    destroy_swapchain_objects(renderer);
    create_swapchain(renderer);
    create_image_views(renderer);
    create_render_pass(renderer);
    create_graphics_pipeline(renderer);
    create_framebuffers(renderer);
    create_command_buffers(renderer);
}

void run_renderer(struct Renderer *renderer)
{
    while (!glfwWindowShouldClose(renderer->window))
    {
        glfwPollEvents();
        
        uint32_t img = 0;
        vkAcquireNextImageKHR(renderer->device, renderer->swapchain, UINT64_MAX,
            renderer->presented_semaphore, NULL, &img);
        
        VkPipelineStageFlagBits waitstgs = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submit = {};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &renderer->command_buffers[img];
        submit.waitSemaphoreCount = 1;
        submit.pWaitDstStageMask = &waitstgs;
        submit.pWaitSemaphores = &renderer->presented_semaphore;
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &renderer->rendered_semaphore;

        vkWaitForFences(renderer->device, 1, &renderer->fences[img], VK_TRUE, UINT64_MAX);
        vkResetFences(renderer->device, 1, &renderer->fences[img]);
        vkQueueSubmit(renderer->graphics_queue, 1, &submit, renderer->fences[img]);

        VkPresentInfoKHR present = {};
        present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = &renderer->rendered_semaphore;
        present.swapchainCount = 1;
        present.pSwapchains = &renderer->swapchain;
        present.pImageIndices = &img;

        vkQueuePresentKHR(renderer->present_queue, &present);
    }

    vkDeviceWaitIdle(renderer->device);
}

void destroy_renderer(struct Renderer *renderer)
{
    destroy_sync_objects(renderer);
    destroy_swapchain_objects(renderer);
    vkDestroyCommandPool(renderer->device, renderer->command_pool, NULL);
    vkDestroyDevice(renderer->device, NULL);
    vkDestroySurfaceKHR(renderer->instance, renderer->surface, NULL);
#ifndef NDEBUG
    destroy_debug_messenger(renderer);
#endif
    vkDestroyInstance(renderer->instance, NULL);
    glfwDestroyWindow(renderer->window);
    glfwTerminate();
    free(renderer);
}