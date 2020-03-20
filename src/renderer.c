#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "device.h"
#include "instance.h"
#include "swapchain_details.h"
#include "debug_messenger.h"
#include "renderer.h"

#define WIDTH 1280
#define HEIGHT 720

static void create_window(struct Renderer *renderer)
{
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        renderer->window = glfwCreateWindow(WIDTH, HEIGHT, "Lindmar", NULL, NULL);
}

static void create_surface(struct Renderer *renderer)
{
        assert_vulkan(glfwCreateWindowSurface(renderer->instance, renderer->window, NULL, &renderer->surface),
                "Failed to create a Vulkan surface!");
}

static void create_swapchain(const struct QueueFamilyIndices *indices, const struct SwapchainDetails *details,
        struct Renderer *renderer)
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

/*
 * renderer->image_views should be cleaned up by destroy_image_views()
 */
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
        for (uint32_t i = 0; i < renderer->image_count; ++i) {
                vkDestroyImageView(renderer->device, renderer->image_views[i], NULL);
        }

        free(renderer->image_views);
}

/*
 * renderer->render_pass should be cleaned up by vkDestroyRenderPass()
 */
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

/* info->module should be cleaned up by vkDestroyShaderModule() */
static void create_shader_info(const char *path, const VkShaderStageFlagBits stage,
        const VkDevice device, VkPipelineShaderStageCreateInfo *info)
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

/*
 * renderer->pipeline_layout should be cleaned up by vkDestroyPipelineLayout()
 * renderer->graphics_pipeline should be cleaned up by vkDestroyPipeline()
 */
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

void create_renderer(struct Renderer *renderer)
{
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

void assert_vulkan(VkResult res, const char *msg)
{
        if (res != VK_SUCCESS) {
                printf("%s\n", msg);
                exit(-1);
        }
}